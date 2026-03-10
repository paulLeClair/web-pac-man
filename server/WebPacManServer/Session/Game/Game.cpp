//
// Created by paull on 2026-03-06.
//
#include "Game.hpp"
#include "../Session.hpp"
#include "../../../protobuf/gen/wpm_packet.pb.h"

namespace pacman
{

    void Game::triggerSound(const SoundType soundType, Session& session)
    {
        std::lock_guard lock(session.mutex);
        sendSoundPacket(soundType, WpmPacketType::TRIGGER_SOUND, session);
    }

    void Game::loopSound(const SoundType soundType, Session& session)
    {
        std::lock_guard lock(session.mutex);
        sendSoundPacket(soundType, WpmPacketType::LOOP_SOUND, session);
    }

    void Game::stopSound(const SoundType soundType, Session& session)
    {
        std::lock_guard lock(session.mutex);
        sendSoundPacket(soundType, WpmPacketType::STOP_SOUND, session);
    }

    void Game::sendSoundPacket(SoundType soundType, WpmPacketType soundControlType, Session& session)
    {
        // very simple, just pack up the appropriate flags
        auto triggerSoundPacketMessage = WpmPacket();

        triggerSoundPacketMessage.set_packettype(static_cast<int32_t>(soundControlType));
        triggerSoundPacketMessage.set_payload(static_cast<int32_t>(soundType));

        std::vector<uint8_t> packetData(triggerSoundPacketMessage.ByteSizeLong() + 1);
        packetData[0] = static_cast<uint8_t>(OutgoingPacketType::SoundControlPacket);
        if (const auto serializeSoundPacketResult = triggerSoundPacketMessage.SerializeToArray(packetData.data() + 1, packetData.size() - 1); !serializeSoundPacketResult)
        {
            // log error
            return;
        }

        outgoingSoundMessageBuffer.consume(packetData.size()); // clear any previous data
        const auto mutableBuffer = outgoingSoundMessageBuffer.prepare(packetData.size());
        boost::asio::buffer_copy(mutableBuffer, boost::asio::buffer(packetData));
        outgoingSoundMessageBuffer.commit(packetData.size());

        session.ws.binary(true);

        boost::asio::dispatch(
            session.ws.get_executor(),
            [&]
            {
                session.ws.async_write(
                    outgoingSoundMessageBuffer.data(),
                    beast::bind_front_handler(&Game::asyncSoundPacketWriteHandler, shared_from_this())
                );
            }
        );

    }

    void Game::asyncSoundPacketWriteHandler(beast::error_code ec, std::size_t bytesTransferred)
    {
        boost::ignore_unused(bytesTransferred);

        if (ec)
        {
            // log!
        }
    }
}
