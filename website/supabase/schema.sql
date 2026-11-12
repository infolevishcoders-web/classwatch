-- ClassWatch Supabase schema
-- Run this in the Supabase SQL editor (Dashboard -> SQL -> New query)

create extension if not exists postgis;
create extension if not exists pgcrypto;

-- ============ profiles ============
create table if not exists public.profiles (
  id uuid primary key references auth.users(id) on delete cascade,
  email text not null unique,
  display_name text,
  auth_provider text default 'email',        -- email | google | apple
  role text not null default 'teacher' check (role in ('teacher','admin','superadmin')),
  is_blocked boolean not null default false,
  block_reason text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- ============ organizations (schools) ============
create table if not exists public.organizations (
  id uuid primary key default gen_random_uuid(),
  owner_id uuid not null references public.profiles(id) on delete cascade,
  name text not null,
  org_type text not null default 'school' check (org_type in ('school','college','training','other')),
  country text,
  district text,
  address text,
  location geography(point, 4326),           -- pinned on map or from Google Maps URL
  location_source text,                       -- 'gps' | 'map_url' | 'manual'
  location_reason text,
  phone text,
  created_at timestamptz not null default now()
);
create index if not exists orgs_location_gix on public.organizations using gist (location);

-- ============ subscription plans (UGX pricing) ============
create table if not exists public.plans (
  code text primary key,                      -- monthly|quarterly|halfyearly|annual|three_year|four_year|five_year
  name text not null,
  duration_months int not null,
  price_ugx int not null,
  device_limit int not null default 3,
  features jsonb not null default '{}'::jsonb,
  active boolean not null default true
);

insert into public.plans (code,name,duration_months,price_ugx,device_limit) values
  ('monthly',      '1 Month',    1,   20000,  3),
  ('quarterly',    '3 Months',   3,   55000,  3),
  ('halfyearly',   '6 Months',   6,  105000,  3),
  ('annual',       '1 Year',     12, 195000,  5),
  ('three_year',   '3 Years',    36, 520000,  8),
  ('four_year',    '4 Years',    48, 650000, 10),
  ('five_year',    '5 Years',    60, 780000, 12)
on conflict (code) do nothing;

-- ============ subscriptions ============
create table if not exists public.subscriptions (
  id uuid primary key default gen_random_uuid(),
  profile_id uuid not null references public.profiles(id) on delete cascade,
  plan_code text not null references public.plans(code),
  status text not null default 'trialing' check (status in ('trialing','active','past_due','cancelled','expired')),
  seats int not null default 3,
  started_at timestamptz not null default now(),
  expires_at timestamptz not null,
  trial_ends_at timestamptz,
  auto_renew boolean not null default false,
  payment_provider text,                       -- pawapat | manual | trial
  payment_ref text,
  created_at timestamptz not null default now()
);
create index if not exists subs_profile_idx on public.subscriptions(profile_id);
create index if not exists subs_expires_idx on public.subscriptions(expires_at);

-- ============ licenses (issued to Master installs) ============
create table if not exists public.licenses (
  id uuid primary key default gen_random_uuid(),
  subscription_id uuid references public.subscriptions(id) on delete set null,
  profile_id uuid not null references public.profiles(id) on delete cascade,
  license_key text not null unique,            -- CW1-<payload>.<sig>
  payload jsonb not null,                      -- email, plan, seats, iat, exp
  seats_used int not null default 0,
  seats_max int not null default 3,
  is_active boolean not null default true,
  issued_at timestamptz not null default now(),
  expires_at timestamptz not null
);
create index if not exists licenses_profile_idx on public.licenses(profile_id);

-- ============ devices (monitored machines) ============
create table if not exists public.devices (
  id uuid primary key default gen_random_uuid(),
  license_id uuid references public.licenses(id) on delete cascade,
  profile_id uuid not null references public.profiles(id) on delete cascade,
  device_fingerprint text not null,            -- machine GUID / hash
  hostname text,
  os text,
  role text not null default 'student' check (role in ('master','student','server')),
  org_id uuid references public.organizations(id) on delete set null,
  last_seen_at timestamptz,
  is_blocked boolean not null default false,
  block_reason text,
  created_at timestamptz not null default now(),
  unique(profile_id, device_fingerprint)
);

-- ============ usage & security events ============
create table if not exists public.usage_events (
  id bigint generated always as identity primary key,
  profile_id uuid references public.profiles(id) on delete cascade,
  device_id uuid references public.devices(id) on delete set null,
  event_type text not null,                    -- login|monitor|control|message|lockdown_violation|...
  detail jsonb,
  created_at timestamptz not null default now()
);
create index if not exists usage_profile_idx on public.usage_events(profile_id, created_at desc);

create table if not exists public.security_events (
  id bigint generated always as identity primary key,
  device_id uuid references public.devices(id) on delete set null,
  profile_id uuid references public.profiles(id) on delete cascade,
  kind text not null,                          -- client_quit_denied|configurator_unlock_failed|...
  detail text,
  local_time timestamptz,
  utc_time timestamptz not null default now(),
  acknowledged boolean not null default false,
  created_at timestamptz not null default now()
);
create index if not exists sec_events_ack_idx on public.security_events(acknowledged) where not acknowledged;

-- ============ OTP codes for login ============
create table if not exists public.otp_codes (
  id uuid primary key default gen_random_uuid(),
  email text not null,
  code_hash text not null,                     -- sha256 of 6-digit code
  purpose text not null default 'login',       -- login|delete_account|admin
  expires_at timestamptz not null,
  attempts int not null default 0,
  consumed boolean not null default false,
  created_at timestamptz not null default now()
);

-- ============ support tickets ============
create table if not exists public.support_tickets (
  id uuid primary key default gen_random_uuid(),
  profile_id uuid references public.profiles(id) on delete set null,
  subject text not null,
  category text not null default 'general',    -- connection|billing|bug|other
  message text not null,
  status text not null default 'open' check (status in ('open','in_progress','resolved','closed')),
  admin_reply text,
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

-- ============ admin users (dashboard super-admins) ============
create table if not exists public.admin_users (
  id uuid primary key default gen_random_uuid(),
  email text not null unique,
  password_hash text not null,                 -- bcrypt/argon2
  display_name text,
  is_active boolean not null default true,
  created_at timestamptz not null default now()
);

-- ============ row level security ============
alter table public.profiles enable row level security;
alter table public.organizations enable row level security;
alter table public.subscriptions enable row level security;
alter table public.licenses enable row level security;
alter table public.devices enable row level security;
alter table public.usage_events enable row level security;
alter table public.security_events enable row level security;
alter table public.support_tickets enable row level security;
alter table public.plans enable row level security;

-- authenticated users can read/write their own rows
create policy "own profile" on public.profiles for all using (auth.uid() = id) with check (auth.uid() = id);
create policy "own orgs" on public.organizations for all using (auth.uid() = owner_id) with check (auth.uid() = owner_id);
create policy "own subs" on public.subscriptions for all using (auth.uid() = profile_id) with check (auth.uid() = profile_id);
create policy "own devices" on public.devices for all using (auth.uid() = profile_id) with check (auth.uid() = profile_id);
create policy "own tickets" on public.support_tickets for all using (auth.uid() = profile_id) with check (auth.uid() = profile_id);
create policy "read plans" on public.plans for select using (true);

-- everything else is accessed through the Render backend with the
-- service-role key (bypasses RLS) - never expose service key client-side.

-- ============ payments (Pawapay mobile money) ============
create table if not exists public.payments (
  id uuid primary key default gen_random_uuid(),
  profile_id uuid not null references public.profiles(id) on delete cascade,
  deposit_id text unique,                     -- Pawapay deposit UUID
  plan_code text not null,
  amount_ugx integer not null,
  currency text not null default 'UGX',
  phone text,
  provider text,
  status text not null default 'pending' check (status in ('pending','completed','failed','expired')),
  license_key text,
  created_at timestamptz not null default now(),
  completed_at timestamptz
);
create index if not exists payments_profile_idx on public.payments(profile_id);
create index if not exists payments_deposit_idx on public.payments(deposit_id);
alter table public.payments enable row level security;
create policy "own payments" on public.payments for select using (auth.uid() = profile_id);
