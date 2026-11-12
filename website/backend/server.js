/**
 * ClassWatch API - licensing, accounts, subscriptions, devices, admin
 *
 * Deploy on Render (Web Service, Node). Required env vars:
 *   SUPABASE_URL, SUPABASE_SERVICE_ROLE_KEY  (Supabase project settings)
 *   JWT_SECRET                               (any long random string)
 *   CLASSWATCH_LICENSE_SECRET                (must match core/src/ClassWatchLicense.cpp)
 *   ADMIN_EMAIL, ADMIN_PASSWORD_HASH         (bcrypt hash of admin password)
 *   SMTP_URL                                 (optional - for OTP emails; without it codes are returned in dev mode)
 *   PAWAPAT_API_TOKEN, PAWAPAT_BASE_URL   (Pawapay mobile money - sandbox by default)
 *   FRONTEND_URL                             (e.g. https://classwatch.netlify.app)
 */

require('dotenv').config();
const express = require('express');
const cors = require('cors');
const crypto = require('crypto');
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const { createClient } = require('@supabase/supabase-js');

const app = express();
app.use(cors({ origin: process.env.FRONTEND_URL || '*' }));
app.use(express.json({ limit: '1mb' }));

const supabase = createClient(
  process.env.SUPABASE_URL || 'https://placeholder.supabase.co',
  process.env.SUPABASE_SERVICE_ROLE_KEY || 'placeholder'
);

const JWT_SECRET = process.env.JWT_SECRET || 'dev-secret-change-me';
const LICENSE_SECRET = process.env.CLASSWATCH_LICENSE_SECRET ||
  'ClsW@tch-L1c3ns1ng-v1#4mQ2vL8nP4xK9';
const TRIAL_DAYS = 7;
const TRIAL_SEATS = 3;

// ---------- helpers ----------

const b64url = (buf) =>
  Buffer.from(buf).toString('base64')
    .replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/, '');

function signPayload(payloadObj) {
  const payload = Buffer.from(JSON.stringify(payloadObj));
  const sig = crypto.createHmac('sha256', LICENSE_SECRET).update(payload).digest();
  return `CW1-${b64url(payload)}.${b64url(sig)}`;
}

function signJwt(payload, expiresIn = '7d') {
  return jwt.sign(payload, JWT_SECRET, { expiresIn });
}

function authUser(req, res, next) {
  const h = req.headers.authorization || '';
  const token = h.startsWith('Bearer ') ? h.slice(7) : null;
  if (!token) return res.status(401).json({ error: 'missing token' });
  try {
    req.user = jwt.verify(token, JWT_SECRET);
    next();
  } catch {
    res.status(401).json({ error: 'invalid token' });
  }
}

async function getOrCreateProfile({ id, email, provider = 'email', displayName }) {
  const { data: existing } = await supabase
    .from('profiles').select('*').eq('id', id).maybeSingle();
  if (existing) return existing;
  const { data, error } = await supabase.from('profiles').insert({
    id, email, auth_provider: provider, display_name: displayName || email.split('@')[0],
  }).select().single();
  if (error) throw error;
  return data;
}

async function activeSubscription(profileId) {
  const { data } = await supabase.from('subscriptions')
    .select('*, plans(*)').eq('profile_id', profileId)
    .in('status', ['trialing', 'active']).order('expires_at', { ascending: false })
    .limit(1).maybeSingle();
  if (!data) return null;
  if (new Date(data.expires_at) < new Date()) {
    await supabase.from('subscriptions').update({ status: 'expired' }).eq('id', data.id);
    return null;
  }
  return data;
}

async function issueLicense(profile, subscription) {
  const plan = subscription?.plans || {};
  const seats = subscription?.seats || plan.device_limit || TRIAL_SEATS;
  const days = plan.duration_months ? plan.duration_months * 30 : 30;
  const exp = new Date(Date.now() + days * 864e5).toISOString();
  const payload = {
    v: 1, email: profile.email, plan: subscription?.plan_code || 'trial',
    seats, iat: new Date().toISOString(), exp,
    lic: 'CW-' + crypto.randomBytes(4).toString('hex').toUpperCase(),
  };
  const key = signPayload(payload);
  const { data: lic } = await supabase.from('licenses').insert({
    profile_id: profile.id, subscription_id: subscription?.id || null,
    license_key: key, payload, seats_max: seats, expires_at: exp,
  }).select().single();
  return { key, license: lic, payload };
}

function haversineKm(lat1, lon1, lat2, lon2) {
  const R = 6371, dLat = (lat2 - lat1) * Math.PI / 180, dLon = (lon2 - lon1) * Math.PI / 180;
  const a = Math.sin(dLat / 2) ** 2 +
    Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) * Math.sin(dLon / 2) ** 2;
  return 2 * R * Math.asin(Math.sqrt(a));
}

// ---------- public: plans & trial ----------

app.get('/api/plans', async (req, res) => {
  const { data } = await supabase.from('plans').select('*').eq('active', true).order('price_ugx');
  res.json({ plans: data || [] });
});

app.post('/api/trial/start', async (req, res) => {
  const { email, org_name, org_type, country, district, address, location, location_reason } = req.body || {};
  if (!email || !/^[^@\s]+@[^@\s]+\.[^@\s]+$/.test(email)) {
    return res.status(400).json({ error: 'valid email required' });
  }
  // one trial per email
  const { data: existing } = await supabase.from('profiles').select('id').eq('email', email).maybeSingle();
  if (existing) return res.status(409).json({ error: 'email already registered - please log in' });

  // create auth user via admin API (Supabase email signup)
  const { data: authData, error: authErr } = await supabase.auth.admin.createUser({
    email, email_confirm: true, user_metadata: { org_name, org_type },
  });
  if (authErr) return res.status(400).json({ error: authErr.message });

  const profile = await getOrCreateProfile({ id: authData.user.id, email });

  if (org_name) {
    await supabase.from('organizations').insert({
      owner_id: profile.id, name: org_name, org_type: org_type || 'school',
      country, district, address,
      location: location ? `SRID=4326;POINT(${location.lng} ${location.lat})` : null,
      location_source: location ? 'map_url' : null,
      location_reason: location_reason || null,
    });
  }

  const expires = new Date(Date.now() + TRIAL_DAYS * 864e5).toISOString();
  const { data: sub } = await supabase.from('subscriptions').insert({
    profile_id: profile.id, plan_code: 'trial', status: 'trialing',
    seats: TRIAL_SEATS, expires_at: expires, trial_ends_at: expires,
    payment_provider: 'trial',
  }).select('*, plans(*)').single();

  const { key } = await issueLicense(profile, { ...sub, plans: { duration_months: 0, device_limit: TRIAL_SEATS } });
  const token = signJwt({ sub: profile.id, email, role: profile.role });
  res.json({ token, profile, subscription: sub, license_key: key, trial_days: TRIAL_DAYS });
});

// ---------- auth: login + OTP ----------

app.post('/api/auth/login', async (req, res) => {
  const { email, password } = req.body || {};
  if (!email || !password) return res.status(400).json({ error: 'email and password required' });
  const { data, error } = await supabase.auth.signInWithPassword({ email, password });
  if (error) return res.status(401).json({ error: 'invalid credentials' });
  const profile = await getOrCreateProfile({ id: data.user.id, email });
  if (profile.is_blocked) return res.status(403).json({ error: 'account blocked: ' + (profile.block_reason || '') });
  const token = signJwt({ sub: profile.id, email, role: profile.role });
  res.json({ token, profile });
});

app.post('/api/auth/otp/send', async (req, res) => {
  const { email } = req.body || {};
  if (!email) return res.status(400).json({ error: 'email required' });
  const code = String(crypto.randomInt(100000, 999999));
  const hash = crypto.createHash('sha256').update(code + JWT_SECRET).digest('hex');
  await supabase.from('otp_codes').insert({
    email, code_hash: hash, purpose: 'login',
    expires_at: new Date(Date.now() + 10 * 60e3).toISOString(),
  });
  // TODO: send via SMTP (nodemailer) - for now return code in dev mode
  if (!process.env.SMTP_URL) return res.json({ sent: true, dev_code: code });
  res.json({ sent: true });
});

app.post('/api/auth/otp/verify', async (req, res) => {
  const { email, code } = req.body || {};
  const hash = crypto.createHash('sha256').update(String(code) + JWT_SECRET).digest('hex');
  const { data: otp } = await supabase.from('otp_codes')
    .select('*').eq('email', email).eq('code_hash', hash)
    .eq('consumed', false).order('created_at', { ascending: false }).limit(1).maybeSingle();
  if (!otp || new Date(otp.expires_at) < new Date() || otp.attempts >= 5) {
    return res.status(401).json({ error: 'invalid or expired code' });
  }
  await supabase.from('otp_codes').update({ consumed: true }).eq('id', otp.id);
  // find or create user
  const { data: prof } = await supabase.from('profiles').select('*').eq('email', email).maybeSingle();
  if (!prof) return res.status(404).json({ error: 'no account for this email - sign up first' });
  if (prof.is_blocked) return res.status(403).json({ error: 'account blocked' });
  const token = signJwt({ sub: prof.id, email, role: prof.role });
  res.json({ token, profile: prof });
});

// ---------- auth: OAuth (Google/Apple via Supabase) ----------
// Frontend sends the Supabase access_token after signInWithOAuth redirect;
// we verify it against Supabase, then issue a ClassWatch JWT.

app.post('/api/auth/oauth', async (req, res) => {
  const { access_token } = req.body || {};
  if (!access_token) return res.status(400).json({ error: 'access_token required' });

  const { data: { user }, error } = await supabase.auth.getUser(access_token);
  if (error || !user || !user.email) return res.status(401).json({ error: 'invalid session' });

  const profile = await getOrCreateProfile({
    id: user.id, email: user.email,
    provider: user.app_metadata?.provider || 'oauth',
    displayName: user.user_metadata?.full_name || user.user_metadata?.name,
  });
  if (profile.is_blocked) return res.status(403).json({ error: 'account blocked' });

  // new OAuth users get the same free trial as email signup
  let sub = await activeSubscription(profile.id);
  let license_key = null;
  if (!sub) {
    const expires = new Date(Date.now() + TRIAL_DAYS * 864e5).toISOString();
    const { data: newSub } = await supabase.from('subscriptions').insert({
      profile_id: profile.id, plan_code: 'trial', status: 'trialing',
      seats: TRIAL_SEATS, expires_at: expires, trial_ends_at: expires,
      payment_provider: 'trial',
    }).select('*, plans(*)').single();
    sub = newSub;
    const { key } = await issueLicense(profile, { ...sub, plans: { duration_months: 0, device_limit: TRIAL_SEATS } });
    license_key = key;
  }

  const token = signJwt({ sub: profile.id, email: profile.email, role: profile.role });
  res.json({ token, profile, subscription: sub, license_key, trial_days: TRIAL_SEATS });
});

// ---------- devices (3 per license on trial) ----------

app.post('/api/devices/register', authUser, async (req, res) => {
  const { fingerprint, hostname, os, role = 'student', org_id } = req.body || {};
  if (!fingerprint) return res.status(400).json({ error: 'fingerprint required' });
  const profileId = req.user.sub;

  // seat enforcement: count active devices vs subscription seats
  const sub = await activeSubscription(profileId);
  const seats = sub?.seats || TRIAL_SEATS;
  const { count } = await supabase.from('devices')
    .select('id', { count: 'exact', head: true })
    .eq('profile_id', profileId).eq('is_blocked', false);
  const { data: dup } = await supabase.from('devices')
    .select('*').eq('profile_id', profileId).eq('device_fingerprint', fingerprint).maybeSingle();
  if (!dup && (count || 0) >= seats) {
    return res.status(403).json({ error: `device limit reached (${seats}) - upgrade your plan` });
  }

  const { data: device, error } = await supabase.from('devices').upsert({
    profile_id: profileId, device_fingerprint: fingerprint, hostname, os, role,
    org_id: org_id || null, last_seen_at: new Date().toISOString(),
  }, { onConflict: 'profile_id,device_fingerprint' }).select().single();
  if (error) return res.status(500).json({ error: error.message });
  res.json({ device });
});

app.get('/api/devices', authUser, async (req, res) => {
  const { data } = await supabase.from('devices')
    .select('*, organizations(name)')
    .eq('profile_id', req.user.sub).order('last_seen_at', { ascending: false });
  res.json({ devices: data || [] });
});

app.post('/api/devices/:id/block', authUser, async (req, res) => {
  const { reason } = req.body || {};
  const { data } = await supabase.from('devices').update({
    is_blocked: true, block_reason: reason || 'blocked by admin',
  }).eq('id', req.params.id).eq('profile_id', req.user.sub).select().single();
  res.json({ device: data });
});

app.post('/api/devices/:id/unblock', authUser, async (req, res) => {
  const { data } = await supabase.from('devices').update({ is_blocked: false, block_reason: null })
    .eq('id', req.params.id).eq('profile_id', req.user.sub).select().single();
  res.json({ device: data });
});

// ---------- subscriptions & checkout ----------

app.get('/api/subscription/status', authUser, async (req, res) => {
  const sub = await activeSubscription(req.user.sub);
  const { data: history } = await supabase.from('subscriptions')
    .select('*, plans(*)').eq('profile_id', req.user.sub)
    .order('created_at', { ascending: false }).limit(10);
  const { data: licenses } = await supabase.from('licenses')
    .select('license_key, seats_max, seats_used, expires_at, is_active')
    .eq('profile_id', req.user.sub).eq('is_active', true);
  res.json({ active: sub, history: history || [], licenses: licenses || [] });
});

// ---------- Pawapay mobile money (deposits) ----------
// Env: PAWAPAT_API_TOKEN (bearer token from dashboard.pawapay.io),
//      PAWAPAT_BASE_URL (default sandbox - use https://api.pawapay.io for live)

const PAWAPAT_BASE = (process.env.PAWAPAT_BASE_URL || 'https://api.sandbox.pawapay.io').replace(/\/$/, '');
const PAWAPAT_TOKEN = process.env.PAWAPAT_API_TOKEN || process.env.PAWAPAT_API_KEY || '';

async function pawapay(path, { method = 'GET', body } = {}) {
  const res = await fetch(PAWAPAT_BASE + path, {
    method,
    headers: {
      'Content-Type': 'application/json',
      'Authorization': 'Bearer ' + PAWAPAT_TOKEN,
    },
    body: body ? JSON.stringify(body) : undefined,
  });
  const data = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(data.message || data.reason || ('Pawapay HTTP ' + res.status));
  return data;
}

// Uganda mobile-money provider detection from phone number.
// PawaPay provider IDs: MTN_MOMO_UGA, AIRTEL_OAPI_UGA
function detectUgandaProvider(phone) {
  const digits = String(phone || '').replace(/\D/g, '');
  const local = digits.startsWith('256') ? digits.slice(3) : digits;
  if (/^(256)?(77|78|76|39)/.test(digits) || /^(77|78|76|39)/.test(local)) return 'MTN_MOMO_UGA';
  if (/^(256)?(70|75|20)/.test(digits) || /^(70|75|20)/.test(local)) return 'AIRTEL_OAPI_UGA';
  return null;
}

function normalizeUgandaPhone(phone) {
  const digits = String(phone || '').replace(/\D/g, '');
  if (digits.startsWith('256')) return digits;
  if (digits.startsWith('0')) return '256' + digits.slice(1);
  return '256' + digits;
}

async function activatePaidSubscription(profileId, plan, depositId, reference) {
  // idempotent: skip if a subscription with this payment ref already exists
  const { data: dup } = await supabase.from('subscriptions')
    .select('id').eq('profile_id', profileId).eq('payment_ref', reference || depositId).maybeSingle();
  if (dup) return { subscription: dup, already: true };

  const expires = new Date(Date.now() + plan.duration_months * 30 * 864e5).toISOString();
  const { data: sub, error } = await supabase.from('subscriptions').insert({
    profile_id: profileId, plan_code: plan.code, status: 'active', seats: plan.device_limit,
    expires_at: expires, payment_provider: 'pawapat', payment_ref: reference || depositId,
  }).select('*, plans(*)').single();
  if (error) throw error;

  const profile = (await supabase.from('profiles').select('*').eq('id', profileId).single()).data;
  const { key } = await issueLicense(profile, sub);
  return { subscription: sub, license_key: key };
}

app.post('/api/subscription/checkout', authUser, async (req, res) => {
  const { plan_code, phone } = req.body || {};
  const { data: plan } = await supabase.from('plans').select('*').eq('code', plan_code).maybeSingle();
  if (!plan) return res.status(400).json({ error: 'unknown plan' });

  if (PAWAPAT_TOKEN) {
    // real Pawapay deposit - customer approves the charge on their phone
    if (!phone) return res.status(400).json({ error: 'mobile money phone number required (e.g. 743199780)' });
    const provider = detectUgandaProvider(phone);
    if (!provider) return res.status(400).json({ error: 'could not detect mobile money provider - use MTN or Airtel number' });

    const depositId = crypto.randomUUID();
    try {
      await pawapay('/v2/deposits', {
        method: 'POST',
        body: {
          depositId,
          amount: String(plan.price_ugx),
          currency: 'UGX',
          payer: {
            type: 'MMO',
            accountDetails: { phoneNumber: normalizeUgandaPhone(phone), provider },
          },
          customerMessage: 'ClassWatch ' + plan.name,
        },
      });
    } catch (e) {
      return res.status(502).json({ error: 'payment provider error: ' + e.message });
    }

    await supabase.from('payments').insert({
      profile_id: req.user.sub, deposit_id: depositId, plan_code,
      amount_ugx: plan.price_ugx, phone: normalizeUgandaPhone(phone), provider,
    });

    return res.json({
      mode: 'pawapay', deposit_id: depositId, plan, amount_ugx: plan.price_ugx,
      status: 'pending', phone: normalizeUgandaPhone(phone), provider,
      instructions: 'Check your phone - approve the ClassWatch payment prompt to complete checkout.',
    });
  }

  // fallback: manual mobile-money payment reference (no Pawapay keys configured)
  const ref = 'CW-' + Date.now().toString(36).toUpperCase();
  res.json({
    reference: ref, plan, amount_ugx: plan.price_ugx,
    instructions: {
      mobile_money: 'Pay via mobile money and quote this reference. Your license activates after confirmation.',
      card: 'Card payments via Pawapay checkout coming soon.',
    },
    status: 'pending_payment',
  });
});

// poll payment status - activates subscription when Pawapay confirms
app.get('/api/payments/:depositId', authUser, async (req, res) => {
  const { data: payment } = await supabase.from('payments')
    .select('*').eq('deposit_id', req.params.depositId).eq('profile_id', req.user.sub).maybeSingle();
  if (!payment) return res.status(404).json({ error: 'payment not found' });
  if (payment.status === 'completed') {
    return res.json({ status: 'completed', license_key: payment.license_key });
  }
  if (!PAWAPAT_TOKEN) return res.json({ status: payment.status });

  try {
    const dep = await pawapay('/v2/deposits/' + req.params.depositId);
    const pawapayStatus = dep.status || dep.deposit?.status;
    if (pawapayStatus === 'COMPLETED') {
      const { data: plan } = await supabase.from('plans').select('*').eq('code', payment.plan_code).single();
      const { license_key } = await activatePaidSubscription(req.user.sub, plan, payment.deposit_id, payment.deposit_id);
      await supabase.from('payments').update({
        status: 'completed', license_key, completed_at: new Date().toISOString(),
      }).eq('id', payment.id);
      return res.json({ status: 'completed', license_key });
    }
    if (['FAILED', 'REJECTED', 'EXPIRED'].includes(pawapayStatus)) {
      await supabase.from('payments').update({ status: 'failed' }).eq('id', payment.id);
      return res.json({ status: 'failed' });
    }
    return res.json({ status: 'pending' });
  } catch (e) {
    return res.status(502).json({ error: 'could not check payment status: ' + e.message });
  }
});

// Pawapay callback - they POST deposit status updates here
app.post('/api/webhooks/pawapat', async (req, res) => {
  const payload = req.body || {};
  const depositId = payload.depositId || payload.id;
  const status = payload.status;
  if (!depositId) return res.json({ received: true });

  const { data: payment } = await supabase.from('payments')
    .select('*').eq('deposit_id', depositId).maybeSingle();
  if (!payment) return res.json({ received: true }); // unknown deposit - ignore

  if (status === 'COMPLETED' && payment.status !== 'completed') {
    const { data: plan } = await supabase.from('plans').select('*').eq('code', payment.plan_code).single();
    const { license_key } = await activatePaidSubscription(payment.profile_id, plan, depositId, depositId);
    await supabase.from('payments').update({
      status: 'completed', license_key, completed_at: new Date().toISOString(),
    }).eq('id', payment.id);
  } else if (['FAILED', 'REJECTED', 'EXPIRED'].includes(status)) {
    await supabase.from('payments').update({ status: 'failed' }).eq('id', payment.id);
  }
  res.json({ received: true });
});

// ---------- account management (danger zone) ----------

app.post('/api/account/delete-request', authUser, async (req, res) => {
  const { reason, confirm_text } = req.body || {};
  if (confirm_text !== 'DELETE') return res.status(400).json({ error: 'type DELETE to confirm' });
  // marks profile for admin-confirmed deletion; superadmin confirms in dashboard
  await supabase.from('profiles').update({ block_reason: 'delete_requested: ' + (reason || '') }).eq('id', req.user.sub);
  await supabase.from('usage_events').insert({
    profile_id: req.user.sub, event_type: 'account_delete_request', detail: { reason },
  });
  res.json({ status: 'deletion requested - an administrator will confirm and your data will be permanently removed' });
});

app.post('/api/account/delete-confirm', async (req, res) => {
  // called by admin dashboard after reviewing the request
  const { profile_id, admin_token } = req.body || {};
  if (!admin_token) return res.status(401).json({ error: 'admin token required' });
  try { jwt.verify(admin_token, JWT_SECRET); } catch { return res.status(401).json({ error: 'bad token' }); }
  await supabase.from('profiles').delete().eq('id', profile_id); // cascades everywhere
  res.json({ status: 'deleted' });
});

// ---------- geo: schools near me ----------

app.get('/api/geo/nearby', authUser, async (req, res) => {
  const lat = parseFloat(req.query.lat), lng = parseFloat(req.query.lng), radius = parseFloat(req.query.radius || 50);
  if (isNaN(lat) || isNaN(lng)) return res.status(400).json({ error: 'lat/lng required' });
  const { data: orgs } = await supabase.from('organizations')
    .select('id,name,org_type,country,district,address,location_reason,created_at')
    .not('location', 'is', null);
  const nearby = (orgs || []).map(o => {
    // location comes back as WKT/EWKB depending on driver - parse POINT(lng lat)
    const m = /POINT\(([-\d.]+) ([-\d.]+)\)/.exec(JSON.stringify(o.location));
    if (!m) return null;
    const d = haversineKm(lat, lng, parseFloat(m[2]), parseFloat(m[1]));
    return d <= radius ? { ...o, distance_km: Math.round(d * 10) / 10, location: undefined } : null;
  }).filter(Boolean).sort((a, b) => a.distance_km - b.distance_km);
  res.json({ nearby });
});

// ---------- support tickets ----------

app.post('/api/support/tickets', authUser, async (req, res) => {
  const { subject, category, message } = req.body || {};
  if (!subject || !message) return res.status(400).json({ error: 'subject and message required' });
  const { data } = await supabase.from('support_tickets').insert({
    profile_id: req.user.sub, subject, category: category || 'general', message,
  }).select().single();
  res.json({ ticket: data });
});

app.get('/api/support/tickets', authUser, async (req, res) => {
  const { data } = await supabase.from('support_tickets')
    .select('*').eq('profile_id', req.user.sub).order('created_at', { ascending: false });
  res.json({ tickets: data || [] });
});

// ---------- security events (devices report lockdown violations) ----------

app.post('/api/security/events', authUser, async (req, res) => {
  const events = Array.isArray(req.body?.events) ? req.body.events : [req.body];
  const rows = events.filter(e => e && e.kind).map(e => ({
    profile_id: req.user.sub, device_id: e.device_id || null,
    kind: e.kind, detail: e.detail || null,
    local_time: e.localTime || null, utc_time: e.utcTime || new Date().toISOString(),
  }));
  if (rows.length) await supabase.from('security_events').insert(rows);
  res.json({ recorded: rows.length });
});

// ---------- admin dashboard API ----------

function adminAuth(req, res, next) {
  const token = (req.headers.authorization || '').replace('Bearer ', '');
  try {
    const payload = jwt.verify(token, JWT_SECRET);
    if (payload.role !== 'admin' && payload.role !== 'superadmin') throw new Error();
    req.admin = payload; next();
  } catch { res.status(401).json({ error: 'admin only' }); }
}

app.post('/api/admin/login', async (req, res) => {
  const { email, password } = req.body || {};
  // verify against env admin OR admin_users table
  if (email === process.env.ADMIN_EMAIL &&
      await bcrypt.compare(password, process.env.ADMIN_PASSWORD_HASH || '')) {
    return res.json({ token: signJwt({ email, role: 'superadmin' }, '12h') });
  }
  const { data: admin } = await supabase.from('admin_users').select('*').eq('email', email).maybeSingle();
  if (admin && admin.is_active && await bcrypt.compare(password, admin.password_hash)) {
    return res.json({ token: signJwt({ email, role: 'admin', id: admin.id }, '12h') });
  }
  res.status(401).json({ error: 'invalid admin credentials' });
});

app.get('/api/admin/overview', adminAuth, async (req, res) => {
  const [profiles, subs, devices, sec] = await Promise.all([
    supabase.from('profiles').select('id', { count: 'exact', head: true }),
    supabase.from('subscriptions').select('id', { count: 'exact', head: true }).eq('status', 'active'),
    supabase.from('devices').select('id', { count: 'exact', head: true }),
    supabase.from('security_events').select('id', { count: 'exact', head: true }).eq('acknowledged', false),
  ]);
  res.json({
    users: profiles.count || 0, active_subscriptions: subs.count || 0,
    devices: devices.count || 0, unacked_security_events: sec.count || 0,
  });
});

app.get('/api/admin/users', adminAuth, async (req, res) => {
  const { data } = await supabase.from('profiles').select('*').order('created_at', { ascending: false }).limit(200);
  res.json({ users: data || [] });
});

app.post('/api/admin/users/:id/block', adminAuth, async (req, res) => {
  const { reason } = req.body || {};
  const { data } = await supabase.from('profiles').update({
    is_blocked: true, block_reason: reason || 'blocked by administrator',
  }).eq('id', req.params.id).select().single();
  res.json({ user: data });
});

app.post('/api/admin/users/:id/unblock', adminAuth, async (req, res) => {
  const { data } = await supabase.from('profiles').update({ is_blocked: false, block_reason: null })
    .eq('id', req.params.id).select().single();
  res.json({ user: data });
});

app.get('/api/admin/security-events', adminAuth, async (req, res) => {
  const { data } = await supabase.from('security_events')
    .select('*, profiles(email), devices(hostname)')
    .order('created_at', { ascending: false }).limit(200);
  res.json({ events: data || [] });
});

app.post('/api/admin/security-events/:id/ack', adminAuth, async (req, res) => {
  await supabase.from('security_events').update({ acknowledged: true }).eq('id', req.params.id);
  res.json({ ok: true });
});

app.get('/api/admin/subscriptions', adminAuth, async (req, res) => {
  const { data } = await supabase.from('subscriptions')
    .select('*, profiles(email,display_name), plans(name,price_ugx)')
    .order('created_at', { ascending: false }).limit(200);
  res.json({ subscriptions: data || [] });
});

app.get('/api/admin/geo', adminAuth, async (req, res) => {
  const { data } = await supabase.from('organizations')
    .select('id,name,org_type,country,district,address,location,created_at')
    .not('location', 'is', null);
  res.json({ organizations: data || [] });
});

app.get('/api/admin/tickets', adminAuth, async (req, res) => {
  const { data } = await supabase.from('support_tickets')
    .select('*, profiles(email)')
    .order('created_at', { ascending: false }).limit(200);
  res.json({ tickets: data || [] });
});

app.post('/api/admin/tickets/:id/reply', adminAuth, async (req, res) => {
  const { reply, status } = req.body || {};
  const { data } = await supabase.from('support_tickets').update({
    admin_reply: reply, status: status || 'in_progress', updated_at: new Date().toISOString(),
  }).eq('id', req.params.id).select().single();
  res.json({ ticket: data });
});

// usage summary for the logged-in user's dashboard
app.get('/api/usage/summary', authUser, async (req, res) => {
  const since = new Date(Date.now() - 30 * 864e5).toISOString();
  const { data: events } = await supabase.from('usage_events')
    .select('event_type,created_at').eq('profile_id', req.user.sub)
    .gte('created_at', since).order('created_at', { ascending: false }).limit(500);
  const byDay = {};
  (events || []).forEach(e => {
    const d = e.created_at.slice(0, 10);
    byDay[d] = (byDay[d] || 0) + 1;
  });
  const sub = await activeSubscription(req.user.sub);
  res.json({
    events_30d: (events || []).length,
    by_day: byDay,
    subscription_expires: sub?.expires_at || null,
    seats: sub?.seats || TRIAL_SEATS,
  });
});

app.post('/api/usage/events', authUser, async (req, res) => {
  const events = Array.isArray(req.body?.events) ? req.body.events : [req.body];
  const rows = events.filter(e => e && e.event_type).map(e => ({
    profile_id: req.user.sub, device_id: e.device_id || null,
    event_type: e.event_type, detail: e.detail || null,
  }));
  if (rows.length) await supabase.from('usage_events').insert(rows);
  res.json({ recorded: rows.length });
});

// ---------- health ----------
app.get('/api/health', (req, res) => res.json({ ok: true, service: 'classwatch-api', time: new Date().toISOString() }));

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`ClassWatch API listening on :${PORT}`));
