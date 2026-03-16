#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <google/protobuf/text_format.h>

#include "kinbot/world_state/world_state_store.h"

namespace {

using kinbot::common::v1::PrivacyLevel;
using kinbot::event_bus::v1::WorldEvent;
using kinbot::event_bus::v1::WorldEventType;
using kinbot::platform::world_state::ParsePrivacyLevel;
using kinbot::platform::world_state::ParseWorldEventType;
using kinbot::platform::world_state::PrivacyLevelToString;
using kinbot::platform::world_state::WorldEventTypeToString;

int64_t NowMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

bool IsPayloadToken(const std::string& token) {
  return token.find('=') != std::string::npos;
}

void PrintUsage() {
  std::cout
      << "Usage:\n"
      << "  kinbot_world_state_cli EVENT key=value ... [--then EVENT key=value ...]\n\n"
      << "Examples:\n"
      << "  kinbot_world_state_cli PERSON_IDENTIFIED person_id=elder_1 "
         "display_name=Grandma age_group=elder\n"
      << "  kinbot_world_state_cli PERSON_IDENTIFIED person_id=elder_1 --then "
         "FALL_SUSPECTED person_id=elder_1 risk_event_id=fall_001 severity=CRITICAL\n";
}

bool ParseEventSequence(int argc, char** argv, std::vector<WorldEvent>* events,
                        std::string* error) {
  int index = 1;
  int64_t timestamp_ms = NowMs();
  int segment = 0;

  while (index < argc) {
    std::string token = argv[index];
    if (token == "--help" || token == "-h") {
      error->clear();
      return false;
    }

    if (token == "--then") {
      ++index;
      continue;
    }

    WorldEventType event_type = kinbot::event_bus::v1::WORLD_EVENT_TYPE_UNSPECIFIED;
    if (!ParseWorldEventType(token, &event_type)) {
      *error = "unsupported event type: " + token;
      return false;
    }

    WorldEvent event;
    event.set_event_type(event_type);
    event.set_event_id("cli_event_" + std::to_string(++segment));
    event.set_timestamp_ms(timestamp_ms + segment);
    event.set_source_module("cli");
    event.set_privacy_level(kinbot::common::v1::PUBLIC_RUNTIME);
    ++index;

    while (index < argc) {
      token = argv[index];
      if (token == "--then") {
        break;
      }
      if (!IsPayloadToken(token)) {
        *error = "expected key=value payload token, got: " + token;
        return false;
      }

      const auto separator = token.find('=');
      const std::string key = token.substr(0, separator);
      const std::string value = token.substr(separator + 1);

      if (key == "privacy_level") {
        PrivacyLevel level = kinbot::common::v1::PUBLIC_RUNTIME;
        if (!ParsePrivacyLevel(value, &level)) {
          *error = "unsupported privacy_level: " + value;
          return false;
        }
        event.set_privacy_level(level);
      } else if (key == "source_module") {
        event.set_source_module(value);
      } else if (key == "timestamp_ms") {
        try {
          event.set_timestamp_ms(std::stoll(value));
        } catch (const std::exception&) {
          *error = "timestamp_ms must be an integer: " + value;
          return false;
        }
      } else if (key == "event_id") {
        event.set_event_id(value);
      } else {
        (*event.mutable_payload())[key] = value;
      }

      ++index;
    }

    events->push_back(event);
  }

  if (events->empty()) {
    *error = "at least one event is required";
    return false;
  }

  return true;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc <= 1) {
    PrintUsage();
    return 1;
  }

  std::vector<WorldEvent> events;
  std::string error;
  if (!ParseEventSequence(argc, argv, &events, &error)) {
    if (error.empty()) {
      PrintUsage();
      return 0;
    }

    std::cerr << error << "\n";
    PrintUsage();
    return 1;
  }

  kinbot::platform::world_state::WorldStateStore store;
  for (const auto& event : events) {
    const auto result = store.ApplyEvent(event);
    if (!result.applied) {
      std::cerr << "Failed to apply " << WorldEventTypeToString(event.event_type())
                << ": " << result.message << "\n";
      return 2;
    }

    std::cout << "Applied " << WorldEventTypeToString(event.event_type())
              << " (privacy=" << PrivacyLevelToString(event.privacy_level())
              << ")\n";
  }

  std::string rendered_snapshot;
  const bool rendered =
      google::protobuf::TextFormat::PrintToString(store.snapshot(),
                                                  &rendered_snapshot);
  if (!rendered) {
    std::cerr << "Failed to render WorldStateSnapshot\n";
    return 3;
  }
  std::cout << "\nFinal WorldStateSnapshot\n";
  std::cout << rendered_snapshot;
  return 0;
}
