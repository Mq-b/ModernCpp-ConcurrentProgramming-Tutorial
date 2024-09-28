#include "test/AduioPlayer.h"

AudioPlayer audioPlayer;

int main(){
    audioPlayer.addAudioPath(AudioPlayer::soundResources[4]);
    audioPlayer.addAudioPath(AudioPlayer::soundResources[5]);
    audioPlayer.addAudioPath(AudioPlayer::soundResources[6]);
    audioPlayer.addAudioPath(AudioPlayer::soundResources[7]);

    std::thread t{ []{
        std::this_thread::sleep_for(1s);
        audioPlayer.addAudioPath(AudioPlayer::soundResources[1]);
    } };
    std::thread t2{ []{
        audioPlayer.addAudioPath(AudioPlayer::soundResources[0]);
    } };

    std::cout << "乐\n";

    t.join();
    t2.join();

    std::cout << "end\n";
}