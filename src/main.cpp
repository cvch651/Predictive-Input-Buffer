// Predictive Input Buffer - recreacion basada en el comportamiento observado.
// No es el codigo original. Requiere validacion con los bindings de tu SDK.

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <deque>

using namespace geode::prelude;

namespace pib {
    struct Stage { int all; int wave; };
    constexpr Stage STAGES[4] = {{2, 2}, {4, 4}, {8, 8}, {16, 8}};
    constexpr int OFF = 4;
    inline int g_stage = 0; // 0..3 = stages 1..4, 4 = OFF

    inline bool verbose() {
        return Mod::get()->getSettingValue<bool>("verbose-log");
    }

    inline void logStage() {
        if (g_stage == OFF) {
            log::info("[InputBuffer] F6 Cycle -> OFF (100% Raw Input)");
            return;
        }
        auto s = STAGES[g_stage];
        log::info("[InputBuffer] F6 Cycle -> STAGE {}: {} Ticks All | {} Ticks Wave ({:.1f}f @ 60fps)",
            g_stage + 1, s.all, s.wave, s.all / 4.0f);
    }
}

struct PendingInput {
    bool down;
    int button;
    bool isPlayer1;
    int remaining;
};

class $modify(PIBBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<PendingInput> queue;
        bool applying = false;
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        auto* f = m_fields.self();

        if (f->applying || pib::g_stage == pib::OFF) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        auto* player = isPlayer1 ? m_player1 : m_player2;
        bool isWave = player && player->m_isDart;
        auto const stage = pib::STAGES[pib::g_stage];
        int delay = isWave ? stage.wave : stage.all;

        f->queue.push_back({down, button, isPlayer1, delay});

        if (pib::verbose()) {
            log::info("*** BUFFERED +{} TICKS ({:.2f} frames @ 60fps) {}! [Limit: {}] ***",
                delay, delay / 4.0f, down ? "PRESS" : "RELEASE", delay);
        }
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        auto* f = m_fields.self();

        // Process each pending input once per physics-step callback.
        // Inputs are kept in insertion order to preserve press/release ordering.
        for (auto it = f->queue.begin(); it != f->queue.end();) {
            --it->remaining;
            if (it->remaining <= 0) {
                auto input = *it;
                f->applying = true;
                GJBaseGameLayer::handleButton(input.down, input.button, input.isPlayer1);
                f->applying = false;
                it = f->queue.erase(it);
            } else {
                ++it;
            }
        }

        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(PIBPlayLayer, PlayLayer) {
    void resetLevel() {
        // Avoid leaving queued inputs across attempts.
        auto* base = static_cast<GJBaseGameLayer*>(this);
        static_cast<PIBBaseGameLayer*>(base)->m_fields->queue.clear();
        PlayLayer::resetLevel();
    }
};

class $modify(PIBKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(cocos2d::enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (down && !repeat && key == KEY_F6) {
            pib::g_stage = (pib::g_stage + 1) % 5;
            pib::logStage();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};

$on_mod(Loaded) {
    log::info("[InputBuffer] Predictive Input Buffer loaded");
    log::info("[InputBuffer] Starting Mode: STAGE 1 (2 All / 2 Wave)");
    log::info("[InputBuffer] Press F6 in-level to cycle: 2/2 -> 4/4 -> 8/8 -> 16/8 -> OFF");
}
