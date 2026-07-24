#pragma once

// Reemplazá el contenido de adentro con TU certificado real
const char amazon_root_ca[] = R"EOF(
-----BEGIN CERTIFICATE-----
TU_CERTIFICADO_AQUI
-----END CERTIFICATE-----
)EOF";

const char claim_cert_pem[] = R"EOF(
-----BEGIN CERTIFICATE-----
TU_CERTIFICADO_AQUI
-----END CERTIFICATE-----
)EOF";

const char claim_private_key[] = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
TU_CERTIFICADO_AQUI
-----END RSA PRIVATE KEY-----
)EOF";