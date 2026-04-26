# WU Persistence Backend

This folder contains the production-shaped persistence backend for WU. The first version is intentionally small, but the boundaries are the ones the MMO will keep:

- `WU.Api`: HTTP entry point for Unreal servers, tools, and eventually login/character selection.
- `WU.Application`: use cases and contracts.
- `WU.Domain`: core game persistence types and rules.
- `WU.Infrastructure`: database, cache, and runtime configuration adapters.
- `db/init`: PostgreSQL initialization scripts for local/dev environments.

## Local Development

From this folder:

```powershell
dotnet build WU.Backend.sln
dotnet run --project src/WU.Api
```

The API exposes:

- `GET /health/live`
- `GET /health/ready`
- `GET /api/backend/manifest`
- `POST /api/auth/dev-login`
- `GET /api/auth/me`
- `GET /api/characters/schema`
- `POST /api/characters`
- `GET /api/accounts/{accountId}/realms/{realmId}/characters`

## Ubuntu Runtime

On the Ubuntu laptop, this backend is meant to run with Docker Compose:

```bash
docker compose up --build
```

That starts:

- `wu-api` on port `5080`
- `postgres` on host port `5433`
- `redis` on port `6379`

The first persistence flow is character creation:

```text
POST /api/characters
```

Example body:

```json
{
  "accountId": "00000000-0000-0000-0000-000000000101",
  "realmId": "00000000-0000-0000-0000-000000000001",
  "name": "Raven",
  "race": "Halfblood",
  "sex": "Female",
  "appearance": {
    "skinPresetIndex": 0,
    "hairStyleIndex": 0,
    "hairColorIndex": 0
  }
}
```

Read the dev account's characters:

```bash
curl http://localhost:5080/api/accounts/00000000-0000-0000-0000-000000000101/realms/00000000-0000-0000-0000-000000000001/characters
```

## Development Login

The backend has a development-only login path so we can stop hand-copying seeded account IDs while testing character select:

```bash
curl -s -X POST http://localhost:5080/api/auth/dev-login && echo
```

That returns a temporary bearer token, the seeded development account, and the local development realm. Use the token with:

```bash
TOKEN="paste-token-here"
curl -s http://localhost:5080/api/auth/me -H "Authorization: Bearer $TOKEN" && echo
```

## Login Preparation

The real login server should keep this API shape, but replace the development-only endpoint with proper credential handling:

- `POST /api/auth/login`: validate credentials and create a session.
- `POST /api/auth/refresh`: rotate a session token.
- `POST /api/auth/logout`: revoke the current session.
- `GET /api/auth/me`: return the current account and available realms.

The Unreal client should eventually authenticate with the backend, receive a short-lived session token, list characters, select one, and then pass the session token to a UE dedicated zone server. The zone server should verify that token with the backend before loading character state.
