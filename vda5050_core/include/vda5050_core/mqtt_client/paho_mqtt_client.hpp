/*
 * Copyright (C) 2025 ROS-Industrial Consortium Asia Pacific
 * Advanced Remanufacturing and Technology Centre
 * A*STAR Research Entities (Co. Registration No. 199702110H)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VDA5050_CORE__MQTT_CLIENT__PAHO_MQTT_CLIENT_HPP_
#define VDA5050_CORE__MQTT_CLIENT__PAHO_MQTT_CLIENT_HPP_

#include <mqtt/async_client.h>
#include <mqtt/callback.h>
#include <mqtt/connect_options.h>
#include <mqtt/iaction_listener.h>
#include <mqtt/ssl_options.h>
#include <mqtt/will_options.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <thread>

#include "vda5050_core/mqtt_client/mqtt_client_interface.hpp"

namespace vda5050_core {

namespace mqtt_client {

class PahoMqttClient;

//=============================================================================
class MqttActionListener : public mqtt::iaction_listener
{
public:
  /// \brief Constructor for MqttActionListener
  MqttActionListener();

protected:
  // Documentation inherited from mqtt::iaction_listener
  void on_failure(const mqtt::token& tok) override;

  // Documentation inherited from mqtt::iaction_listener
  void on_success(const mqtt::token& tok) override;
};

//=============================================================================
class MqttCallback : public mqtt::callback
{
public:
  /// \brief Constructor for MqttCallback
  explicit MqttCallback(PahoMqttClient& parent);

protected:
  // Documentation inherited from mqtt::callback
  void connected(const std::string& cause) override;

  // Documentation inherited from mqtt::callback
  void connection_lost(const std::string& cause) override;

  // Documentation inherited from mqtt::callback
  void message_arrived(mqtt::const_message_ptr msg) override;

  // Documentation inherited from mqtt::callback
  void delivery_complete(mqtt::delivery_token_ptr tok) override;

private:
  /// \brief Reference to the MQTT client interface
  PahoMqttClient& parent_;
};

//=============================================================================
class PahoMqttClient : public MqttClientInterface
{
public:
  struct Subscription
  {
    MessageHandler handler;
    int qos;
  };

  /// \brief Create a shared pointer to PahoMqttClient
  ///
  /// \param broker_address Address of the MQTT broker
  /// \param client_id ID of the MQTT client
  ///
  /// \return Shared pointer to Paho MQTT client
  static std::shared_ptr<PahoMqttClient> make(
    const std::string& broker_address, const std::string& client_id);

  /// \brief Destructor for PahoMqttClient
  ~PahoMqttClient();

  PahoMqttClient(const PahoMqttClient&) = delete;
  PahoMqttClient& operator=(const PahoMqttClient&) = delete;
  PahoMqttClient(PahoMqttClient&&) = delete;
  PahoMqttClient& operator=(PahoMqttClient&&) = delete;

  // Documentation inherited from MqttClientInterface
  void connect() override;

  // Documentation inherited from MqttClientInterface
  void disconnect() override;

  // Documentation inherited from MqttClientInterface
  bool connected() override;

  // Documentation inherited from MqttClientInterface
  void publish(
    const std::string& topic, const std::string& message, int qos,
    bool retain = false) override;

  // Documentation inherited from MqttClientInterface
  void subscribe(
    const std::string& topic, MessageHandler handler, int qos) override;

  // Documentation inherited from MqttClientInterface
  void unsubscribe(const std::string& topic) override;

  // Documentation inherited from MqttClientInterface
  void set_will(
    const std::string& topic, const std::string& message, int qos) override;

  /// \brief Get a mutable reference to Paho configuration options
  ///
  /// \return Mutable reference to Paho configuration options
  mqtt::connect_options& connect_options();

  /// \brief Set user credentials for MQTT connection
  ///
  /// \param username Username for MQTT connection
  /// \param password Password for MQTT connection
  void set_user_credentials(const std::string& username, const std::string& password);

  /// \brief Set the trust store for SSL connections
  ///
  /// \param trust_store Path to the trust store file in PEM format
  void set_trust_store(const std::string& trust_store);

  /// \brief Set the client key store for SSL connections
  ///
  /// \param key_store Path to the key store file in PEM format
  void set_key_store(const std::string& key_store);

  /// \brief Set the client private key for SSL connections
  ///
  /// \param private_key Path to the private key file in PEM format
  void set_private_key(const std::string& private_key);

  /// \brief Enable or disable server certificate authentication
  ///
  /// \param enable_server_cert_auth Enable server certificate authentication
  void set_enable_server_cert_auth(bool enable_server_cert_auth);

  /// \brief Configure all SSL options at once
  ///
  /// \param trust_store Path to the trust store file in PEM format
  /// \param key_store Path to the key store file in PEM format
  /// \param private_key Path to the private key file in PEM format
  /// \param enable_server_cert_auth Enable server certificate authentication
  void configure_ssl_options(const std::string& trust_store, const std::string& key_store, const std::string& private_key, bool enable_server_cert_auth);

  friend class MqttCallback;

private:
  /// \brief Private constructor for PahoMqttClient
  ///
  /// \param broker_address Address of the MQTT broker
  /// \param client_id ID of the MQTT client
  PahoMqttClient(
    const std::string& broker_address, const std::string& client_id);

  /// \brief Unique pointer to Paho async client
  std::unique_ptr<mqtt::async_client> client_;

  /// \brief Implementation of action listener
  MqttActionListener action_listener_;

  /// \brief Implementation of callback
  MqttCallback callback_;

  /// \brief List of subscriptions mapped to topics
  std::unordered_map<std::string, Subscription> subscriptions_;

  /// \brief Mutex protecting list of message handlers
  std::mutex handler_mutex_;

  /// \brief MQTT connection options
  mqtt::connect_options conn_options_;

  /// \brief SSL options applied to MQTT connection options
  mqtt::ssl_options ssl_options_;
  /// \brief Flag set before destruction to stop in-flight resubscription threads
  std::atomic<bool> shutdown_{false};
  /// \brief Resubscribe to all cached topics after reconnect
  void resubscribe_topics();

  /// \brief Apply cached SSL options to MQTT connection options
  void apply_ssl_options();
};

}  // namespace mqtt_client
}  // namespace vda5050_core

#endif  // VDA5050_CORE__MQTT_CLIENT__PAHO_MQTT_CLIENT_HPP_
