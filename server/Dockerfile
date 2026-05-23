# Этап сборки: компилируем статический бинарь
FROM golang:1.22-alpine AS builder

WORKDIR /src
COPY go.mod go.sum ./
RUN go mod download

COPY . .
RUN CGO_ENABLED=0 GOOS=linux go build -ldflags="-s -w" -o puf-server .

# Минимальный runtime-образ без Go-тулчейна
FROM scratch

COPY --from=builder /src/puf-server /puf-server
COPY --from=builder /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/

EXPOSE 8080
USER 65534:65534
ENTRYPOINT ["/puf-server"]
