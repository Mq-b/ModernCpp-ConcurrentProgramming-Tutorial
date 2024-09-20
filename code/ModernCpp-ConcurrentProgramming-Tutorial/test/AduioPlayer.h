#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <atomic>
#include <array>
#include <SFML/Audio.hpp>
using namespace std::chrono_literals;

class AudioPlayer {
public:
    AudioPlayer() : stop{ false }, player_thread{ &AudioPlayer::playMusic, this }
    {}

    ~AudioPlayer() {
        // 等待队列中所有音乐播放完毕
        while (!audio_queue.empty()) {
            std::this_thread::sleep_for(50ms);
        }
        stop = true;
        cond.notify_all();
        if (player_thread.joinable()) {
            player_thread.join();
        }
    }

    void addAudioPath(const std::string& path) {
        std::lock_guard<std::mutex> lock{ mtx };
        audio_queue.push(path);
        cond.notify_one();  // 通知线程新的音频
    }

private:
    void playMusic() {
        while (!stop) {
            std::string path;
            {
                std::unique_lock<std::mutex> lock{ mtx };
                cond.wait(lock, [this] { return !audio_queue.empty() || stop; });

                if (audio_queue.empty()) return; // 防止在对象为空时析构出错

                path = audio_queue.front();
                audio_queue.pop();
            }

            if (!music.openFromFile(path)) {
                std::cerr << "无法加载音频文件: " << path << std::endl;
                continue;  // 继续播放下一个音频
            }

            music.play();

            // 等待音频播放完毕
            while (music.getStatus() == sf::SoundSource::Playing) {
                sf::sleep(sf::seconds(0.1f));  // sleep 避免忙等占用CPU
            }
        }
    }

    std::atomic<bool> stop;
    std::thread player_thread;
    std::mutex mtx;
    std::condition_variable cond;
    std::queue<std::string> audio_queue;
    sf::Music music;

public:
    static constexpr std::array soundResources{
        "./sound/01初始化失败.ogg",
        "./sound/02初始化成功.ogg",
        "./sound/03试剂不足，请添加.ogg",
        "./sound/04试剂已失效，请更新.ogg",
        "./sound/05清洗液不足，请添加.ogg",
        "./sound/06废液桶即将装满，请及时清空.ogg",
        "./sound/07废料箱即将装满，请及时清空.ogg",
        "./sound/08激发液A液不足，请添加.ogg",
        "./sound/09激发液B液不足，请添加.ogg",
        "./sound/10反应杯不足，请添加.ogg",
        "./sound/11检测全部完成.ogg"
    };
    enum SoundIndex {
        InitializationFailed,
        InitializationSuccessful,
        ReagentInsufficient,
        ReagentExpired,
        CleaningAgentInsufficient,
        WasteBinAlmostFull,
        WasteContainerAlmostFull,
        LiquidAInsufficient,
        LiquidBInsufficient,
        ReactionCupInsufficient,
        DetectionCompleted,
        SoundCount // 总音频数量，用于计数
    };
};

#endif // AUDIOPLAYER_H
