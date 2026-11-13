// ClassWatch installer gate - Netlify Edge Function
//
// Blocks direct downloads of /files/installers/* unless the visitor has a
// valid ClassWatch session. The website stores the session JWT in the
// `cw_session` cookie (set by assets/app.js on login). This function verifies
// the JWT signature (HS256) and expiry before letting the file through.
//
// REQUIRED Netlify env var: JWT_SECRET (same value as the Render backend).

import type { Context } from "https://edge.netlify.com";

const te = new TextEncoder();

function b64urlToBytes(s: string): Uint8Array {
  const b64 = s.replace(/-/g, "+").replace(/_/g, "/");
  const bin = atob(b64 + "=".repeat((4 - (b64.length % 4)) % 4));
  const out = new Uint8Array(bin.length);
  for (let i = 0; i < bin.length; i++) out[i] = bin.charCodeAt(i);
  return out;
}

function b64urlEncode(buf: ArrayBuffer): string {
  const bytes = new Uint8Array(buf);
  let bin = "";
  for (const b of bytes) bin += String.fromCharCode(b);
  return btoa(bin).replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/, "");
}

function safeEqual(a: string, b: string): boolean {
  if (a.length !== b.length) return false;
  let diff = 0;
  for (let i = 0; i < a.length; i++) diff |= a.charCodeAt(i) ^ b.charCodeAt(i);
  return diff === 0;
}

async function verifyJwt(token: string, secret: string): Promise<boolean> {
  const parts = token.split(".");
  if (parts.length !== 3) return false;
  const [header, payload, signature] = parts;

  let headerJson: any, payloadJson: any;
  try {
    headerJson = JSON.parse(new TextDecoder().decode(b64urlToBytes(header)));
    payloadJson = JSON.parse(new TextDecoder().decode(b64urlToBytes(payload)));
  } catch {
    return false;
  }
  if (headerJson.alg !== "HS256") return false;
  if (!payloadJson.exp || payloadJson.exp * 1000 < Date.now()) return false;

  const key = await crypto.subtle.importKey(
    "raw",
    te.encode(secret),
    { name: "HMAC", hash: "SHA-256" },
    false,
    ["sign"]
  );
  const expected = await crypto.subtle.sign(
    "HMAC",
    key,
    te.encode(`${header}.${payload}`)
  );
  return safeEqual(b64urlEncode(expected), signature);
}

export default async (req: Request, context: Context) => {
  const secret = Deno.env.get("JWT_SECRET") || "";
  const cookie = req.headers.get("cookie") || "";
  const match = /(?:^|;\s*)cw_session=([^;]+)/.exec(cookie);
  const token = match ? decodeURIComponent(match[1]) : "";

  const ok = secret.length > 0 && token.length > 0 && (await verifyJwt(token, secret));
  if (ok) return context.next();

  // not signed in (or signature invalid/expired): send them to log in and
  // back to the download page afterwards
  return Response.redirect(
    new URL("/login.html?next=/download.html", req.url),
    302
  );
};
