#ifndef PAYLOADWRAP_HPP
#define PAYLOADWRAP_HPP

#include <ArduinoJson.h>
#include <memory>  // Para std::unique_ptr
class PayloadWrap {
public:
  PayloadWrap(std::shared_ptr<JsonDocument> payload, DeserializationError error)
    : payload(payload), error(error) {}


  ~PayloadWrap() = default;

  // Métodos para acceder a los datos
  std::shared_ptr<JsonDocument> getJsonDocument() const {
    return payload;
  }

  DeserializationError getDeserializationError() const {
    return error;
  }
private:
  std::shared_ptr<JsonDocument> payload;  // Puntero compartido para JsonDocument
  DeserializationError error;             // Error de deserialización
};

#endif