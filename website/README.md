# ClassWatch Web Platform

- **Frontend (Netlify):** static HTML/CSS/JS in this folder (`netlify.toml` included).
- **Backend API (Render):** `backend/server.js` — Node/Express.
- **Database/Auth (Supabase):** run `supabase/schema.sql` in the Supabase SQL editor.

## 1. Supabase setup

1. Create a project at https://supabase.com
2. SQL Editor → paste and run `supabase/schema.sql`
3. Project Settings → API → copy `Project URL` and `anon public` key (frontend) and `service_role` key (**backend only — never ship it to the browser**)

## 2. Render backend

1. New → Web Service → connect this repo, root directory `website/backend`
2. Build: `npm install` · Start: `node server.js`
3. Environment variables:

| Var | Value |
|---|---|
| `SUPABASE_URL` | your project URL |
| `SUPABASE_SERVICE_ROLE_KEY` | service_role key |
| `JWT_SECRET` | long random string |
| `CLASSWATCH_LICENSE_SECRET` | **must equal** the secret compiled into `core/src/ClassWatchLicense.cpp` |
| `ADMIN_EMAIL` | `info.levishcoders@gmail.com` |
| `ADMIN_PASSWORD_HASH` | `node -e "console.log(require('bcryptjs').hashSync('YOUR-PASSWORD',10))"` |
| `FRONTEND_URL` | your Netlify URL (CORS) |
| `SMTP_URL` | optional SMTP for OTP emails; without it codes are returned in dev mode |
| `PAWAPAT_API_TOKEN` | Pawapay bearer token (dashboard.pawapay.io → API tokens). **Sandbox token = sandbox base URL; live token = live base URL** |
| `PAWAPAT_BASE_URL` | `https://api.sandbox.pawapay.io` for testing, `https://api.pawapay.io` for production |

## 3. Netlify frontend

1. New site → deploy this repo, publish directory `website`
2. Set build command to `echo ok` (static site)
3. In `assets/app.js`, set `API_BASE`, `SUPABASE_URL`, `SUPABASE_ANON_KEY` — or inject via Netlify env vars + a small config snippet.

## 4. Payments (Pawapay — implemented)

Mobile-money checkout is fully wired via the Pawapay **Deposits** API:

1. Dashboard "Pay" → prompts for the customer's MTN/Airtel number → `POST /api/subscription/checkout`
2. Backend calls `POST {PAWAPAT_BASE_URL}/v2/deposits` — the customer gets an approval prompt on their phone
3. Frontend polls `GET /api/payments/:depositId` every 5 s; when Pawapay reports `COMPLETED` the subscription activates and a signed `CW1-…` license key is issued
4. `POST /api/webhooks/pawapat` also activates on Pawapay callbacks (configure the callback URL in the Pawapay dashboard)

Provider detection: MTN prefixes (76/77/78/39) → `MTN_MOMO_UGA`, Airtel (70/75/20) → `AIRTEL_OAPI_UGA`.
Card payments will reuse the same activate-on-confirm flow when enabled in Pawapay.

## 5. License flow

Trial signup and paid subscriptions both call `issueLicense()` which produces an
HMAC-signed key (`CW1-<payload>.<sig>`) verified offline by `ClassWatchLicense` in the
C++ core — Master works without internet once activated.
