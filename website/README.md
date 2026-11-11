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
| `PAWAPAT_API_KEY` / `PAWAPAT_API_SECRET` | add when you receive Pawapat credentials |

## 3. Netlify frontend

1. New site → deploy this repo, publish directory `website`
2. Set build command to `echo ok` (static site)
3. In `assets/app.js`, set `API_BASE`, `SUPABASE_URL`, `SUPABASE_ANON_KEY` — or inject via Netlify env vars + a small config snippet.

## 4. Payments (Pawapat)

`POST /api/subscription/checkout` currently returns a payment reference + UGX amount.
When Pawapat credentials arrive: implement their payment-request API in that handler and
confirm via `POST /api/webhooks/pawapat` (already written — it activates the subscription
and issues a signed `CW1-…` license key). Card payments go through the same webhook flow.

## 5. License flow

Trial signup and paid subscriptions both call `issueLicense()` which produces an
HMAC-signed key (`CW1-<payload>.<sig>`) verified offline by `ClassWatchLicense` in the
C++ core — Master works without internet once activated.
