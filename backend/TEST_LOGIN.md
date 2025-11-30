# How to Test the Login Endpoint

## Step 1: Build the Project

```bash
cd backend
mkdir -p build
cd build
cmake ..
make
```

## Step 2: Start the Server

```bash
cd backend/build
./docs_app
```

The server will start on port 8080. You should see output like:
```
(2018-01-01 00:00:00) [INFO    ] Crow/1.0 server is running on 0.0.0.0:8080
```

## Step 3: Test the Login Endpoint

### Option 1: Using curl (Terminal)

```bash
# Test login endpoint
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"test@example.com","password":"test123"}'

# Expected response: "Login API working" (status 200)
```

### Option 2: Using the test script

```bash
cd backend
./test_login.sh
```

### Option 3: Using a REST client (Postman/Insomnia)

1. **Method**: POST
2. **URL**: `http://localhost:8080/api/auth/login`
3. **Headers**: 
   - `Content-Type: application/json`
4. **Body** (JSON):
   ```json
   {
     "email": "test@example.com",
     "password": "test123"
   }
   ```

### Option 4: Using browser (for GET endpoints only)

Open: `http://localhost:8080/health`

## Expected Responses

### Successful Login (Current Implementation)
- **Status**: 200 OK
- **Response**: `"Login API working"`

### Health Check
- **Status**: 200 OK
- **Response**: 
  ```json
  {
    "status": "ok",
    "service": "docs-backend"
  }
  ```

## Troubleshooting

1. **Port already in use**: Change port in `main.cpp` from 8080 to another port (e.g., 8081)
2. **Connection refused**: Make sure the server is running
3. **Compilation errors**: Fix any build errors first

## Quick Test Commands

```bash
# Health check
curl http://localhost:8080/health

# Login test
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"user@example.com","password":"pass123"}'
```

