#include <iostream>
#include <vector>
#include <span>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "effects.hpp"

int main(void) {

	sf::RenderWindow window(sf::VideoMode(600, 600), "AudioFX", sf::Style::Close);

	sf::SoundBuffer audio;
	audio.loadFromFile("C:/Users/first/Desktop/ATTEMPTED GUITAR/my audio.wav");

	std::vector<sf::Int16> samples(audio.getSamples(), audio.getSamples() + audio.getSampleCount());

	applyReverb(audio, .5);

	sf::Sound sound;
	sound.setBuffer(audio);
	sound.play();
	audio.saveToFile("C:/Users/first/Desktop/ATTEMPTED GUITAR/bitcrush.wav");

	while (window.isOpen()) {
		sf::Event e;
		while (window.pollEvent(e)) {
			switch (e.type) {
			case sf::Event::Closed:
				window.close();
				break;
			}
		}
	}
}
