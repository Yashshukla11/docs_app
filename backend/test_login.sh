#!/bin/bash

# Test script for login endpoint
# Make sure the server is running on port 8080

echo "Testing Login Endpoint..."
echo "=========================="
echo ""

# Test 1: Basic login request
echo "1. Testing POST /api/auth/login with JSON body:"
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"email":"test@example.com","password":"test123"}' \
  -v
echo ""
echo ""

# Test 2: Health check
echo "2. Testing GET /health:"
curl -X GET http://localhost:8080/health \
  -H "Content-Type: application/json" \
  -v
echo ""
echo ""

# Test 3: Login with empty body
echo "3. Testing POST /api/auth/login with empty body:"
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{}' \
  -v
echo ""

