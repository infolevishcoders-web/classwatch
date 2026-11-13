/* ClassWatch website - shared API helpers */

// Configure before deploy: fill these from Supabase project settings + Render URL.
const API_BASE = window.CLASSWATCH_API || 'https://classwatch-website.onrender.com';
const SUPABASE_URL = window.SUPABASE_URL || 'https://zxoavloahtxzitvjzlot.supabase.co';
const SUPABASE_ANON_KEY = window.SUPABASE_ANON_KEY || 'sb_publishable_a46ZJ-qqufDIGSLH1DJOoA_DPIe43wV';

let supabase = null;
try {
  const SB = window.supabase || window.supabasejs;
  if (SB && SUPABASE_URL.includes('http')) {
    supabase = SB.createClient(SUPABASE_URL, SUPABASE_ANON_KEY);
  }
} catch (e) { /* supabase not configured yet */ }

function token() { return localStorage.getItem('cw_token') || ''; }
function setSession(t, profile) {
  if (t) localStorage.setItem('cw_token', t);
  if (profile) localStorage.setItem('cw_profile', JSON.stringify(profile));
}
function clearSession() {
  localStorage.removeItem('cw_token');
  localStorage.removeItem('cw_profile');
}
function profile() {
  try { return JSON.parse(localStorage.getItem('cw_profile') || 'null'); } catch { return null; }
}
function requireAuth(redirect = 'login.html') {
  if (!token()) { location.href = redirect; return false; }
  return true;
}

/* ---------- OAuth session exchange ----------
   Handles the return trip from Google/Apple sign-in: Supabase stores the
   session after the redirect, we trade its access_token for a ClassWatch
   JWT (+ free trial for new users), then reload. */
async function exchangeOAuthAndReload(redirect = 'login.html') {
  try {
    if (supabase) {
      const { data } = await supabase.auth.getSession();
      const at = data && data.session && data.session.access_token;
      if (at) {
        const r = await api('/api/auth/oauth', { method: 'POST', body: { access_token: at } });
        setSession(r.token, r.profile);
        location.reload();
        return;
      }
    }
  } catch (e) { /* fall through to redirect */ }
  location.href = redirect;
}
function isAdmin() {
  try {
    const p = JSON.parse(atob(token().split('.')[1]));
    return p.role === 'admin' || p.role === 'superadmin';
  } catch { return false; }
}

async function api(path, opts = {}) {
  const res = await fetch(API_BASE + path, {
    method: opts.method || 'GET',
    headers: {
      'Content-Type': 'application/json',
      ...(token() ? { Authorization: 'Bearer ' + token() } : {}),
      ...(opts.headers || {}),
    },
    body: opts.body ? JSON.stringify(opts.body) : undefined,
  });
  const data = await res.json().catch(() => ({}));
  if (!res.ok) throw new Error(data.error || ('HTTP ' + res.status));
  return data;
}
function fmtUGX(n) { return 'UGX ' + Number(n || 0).toLocaleString('en-UG'); }
function esc(s) {
  return String(s ?? '').replace(/[&<>"']/g, c =>
    ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
}
function showMsg(el, text, ok) {
  el.textContent = text; el.style.display = 'block';
  el.className = ok ? 'success' : 'error';
}
function hideMsg(el) { el.style.display = 'none'; }

/* ---------- pricing ---------- */
async function renderPlans() {
  const el = document.getElementById('plans');
  if (!el) return;
  try {
    const { plans } = await api('/api/plans');
    el.innerHTML = plans.map(p => `
      <div class="plan ${p.code === 'annual' ? 'hot' : ''}">
        ${p.code === 'annual' ? '<span class="tag">Best value</span>' : ''}
        <h3>${esc(p.name)}</h3>
        <div class="price">${fmtUGX(p.price_ugx)}</div>
        <div class="per">per ${p.duration_months} month${p.duration_months > 1 ? 's' : ''} · ${p.seats || p.device_limit} devices</div>
        <ul><li>${p.device_limit} teacher seats</li><li>All features unlocked</li><li>Priority support</li></ul>
        <a class="btn ${p.code === 'annual' ? 'btn-primary' : 'btn-ghost'}" href="signup.html?plan=${p.code}">Choose</a>
      </div>`).join('');
  } catch {
    el.innerHTML = '<p class="small">Pricing loads from the API once deployed.</p>';
  }
}

/* ---------- Google OAuth (Supabase) ---------- */
async function googleLogin() {
  if (!supabase) { alert('Sign-in provider will be enabled after deployment.'); return; }
  const { data, error } = await supabase.auth.signInWithOAuth({
    provider: 'google',
    options: { redirectTo: location.origin + '/dashboard.html' },
  });
  if (error) alert(error.message);
}
async function appleLogin() {
  if (!supabase) { alert('Sign-in provider will be enabled after deployment.'); return; }
  const { data, error } = await supabase.auth.signInWithOAuth({
    provider: 'apple',
    options: { redirectTo: location.origin + '/dashboard.html' },
  });
  if (error) alert(error.message);
}
