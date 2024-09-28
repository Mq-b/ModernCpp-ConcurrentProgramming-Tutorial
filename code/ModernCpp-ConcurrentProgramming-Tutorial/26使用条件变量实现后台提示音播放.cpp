#include <SFML/Audio.hpp>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <queue>
#include <array>
#include <iostream>
using namespace std::chrono_literals;

class AudioPlayer{
public:
    AudioPlayer() : stop {false}, player_thread{ &AudioPlayer::playMusic, this }
    {}

    ~AudioPlayer(){
        while (!audio_queue.empty()){
            std::this_thread::sleep_for(50ms);
        }
        stop = true;
        cond.notify_all();
        if(player_thread.joinable()){
            player_thread.join();
        }
    }

    void addAudioPath(const std::string& path){
        std::lock_guard<std::mutex> lc{ m };
        audio_queue.push(path);
        cond.notify_one();
    }

private:
    void playMusic(){
        while(!stop){
            std::string path;
            {
                std::unique_lock<std::mutex> lock{ m };
                // 条件不满足，就解锁 unlock，让其它线程得以运行 如果被唤醒了，就会重新获取锁 lock
                cond.wait(lock, [this] {return !audio_queue.empty() || stop; });

                if (audio_queue.empty()) return; // 防止对象为空时出问题

                path = audio_queue.front(); // 取出
                audio_queue.pop();          // 取出后就删除这个元素，表示此元素以及被使用
            }

            if(!music.openFromFile(path)){
                std::cerr << "无法加载音频文件: " << path << std::endl;
                continue;
            }

            music.play(); // 异步 非阻塞

            while(music.getStatus() == sf::SoundSource::Playing){
                sf::sleep(sf::seconds(0.1f)); // sleep 避免忙等 占用 CPU
            }
        }
    }

    std::atomic<bool> stop;              // 控制线程的停止与退出
    std::thread player_thread;           // 后台执行音频播放任务的专用线程
    std::mutex m;                        // 保护共享资源
    std::condition_variable cond;        // 控制线程的等待和唤醒，当有新的任务的时候通知播放线程
    std::queue<std::string> audio_queue; // 音频任务队列，存储待播放的音频文件的路径
    sf::Music music;                     // SFML 音频播放器对象，用来加载播放音频

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
};

AudioPlayer audioPlayer;

int main() {
    audioPlayer.addAudioPath(AudioPlayer::soundResources[4]);
    audioPlayer.addAudioPath(AudioPlayer::soundResources[5]);
    audioPlayer.addAudioPath(AudioPlayer::soundResources[6]);
    audioPlayer.addAudioPath(AudioPlayer::soundResources[7]);

    std::thread t{ [] {
        std::this_thread::sleep_for(1s);
        audioPlayer.addAudioPath(AudioPlayer::soundResources[1]);
    } };
    std::thread t2{ [] {
        audioPlayer.addAudioPath(AudioPlayer::soundResources[0]);
    } };

    std::cout << "乐\n";

    t.join();
    t2.join();

    std::cout << "end\n";
}