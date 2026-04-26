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
- `GET /api/characters/schema`
- `POST /api/characters`

## Ubuntu Runtime

On the Ubuntu laptop, this backend is meant to run with Docker Compose:

```bash
docker compose up --build
```

That starts:

- `wu-api` on port `5080`
- `postgres` on port `5432`
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
