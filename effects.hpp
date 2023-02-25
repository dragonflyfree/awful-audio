#include <vector>
#include <SFML/Audio.hpp>

// I forget if any of this works I just had to edit it to make it look, not horrible idk don't use it

class Delay {
public:
	Delay(float maxDelayTime, float delay, float gain) :
		m_delayBuffer(maxDelayTime * 44100.f + 1.5f, 0),
		m_readIndex(44100.f * (maxDelayTime - delay)),
		m_gain{ gain }
	{ }

	void setDelay(float delay) {
		m_readIndex = m_delayBuffer.size() - 44100.f * delay;
	}

	void setGain(float gain) {
		m_gain = gain;
	}

	void write(int16_t sample) {
		m_delayBuffer[m_writeIndex] = sample + m_gain * m_delayBuffer[m_readIndex];

		m_writeIndex++;
		m_writeIndex %= m_delayBuffer.size();
	}

	int16_t read() {
		return m_delayBuffer[m_readIndex];
	}

	void step() {
		m_readIndex++;
		m_readIndex %= m_delayBuffer.size();
	}

private:
	float m_gain;
	std::size_t m_writeIndex = 0;
	std::size_t m_readIndex;
	std::vector<int16_t> m_delayBuffer;
};

class CombFilter {
public:
	CombFilter(float maxDelayTime, float delay, float gain) :
		m_delayBuffer(maxDelayTime * 44100.f + 1.5f, 0),
		m_readIndex(44100.f * (maxDelayTime - delay)),
		m_gain{ gain }
	{ }

	void setDelay(float delay) {
		m_readIndex = m_delayBuffer.size() - 44100.f * delay;
	}
	
	void setGain(float gain) {
		m_gain = gain;
	}

	void write(int16_t sample) {
		m_delayBuffer[m_writeIndex] = sample + m_gain * m_delayBuffer[m_readIndex];

		m_writeIndex++;
		m_writeIndex %= m_delayBuffer.size();
	}

	int16_t read() {
		return m_delayBuffer[m_readIndex];
	}

	void step() {
		m_readIndex++;
		m_readIndex %= m_delayBuffer.size();
	}

private:
	float m_gain;
	std::size_t m_writeIndex = 0;
	std::size_t m_readIndex;
	std::vector<int16_t> m_delayBuffer;
};

class AllpassFilter {
public:
	AllpassFilter(float maxDelayTime, float delay, float gain) :
		m_delayBuffer(maxDelayTime * 44100.f + 1.5f, 0),
		m_readIndex(44100.f * (maxDelayTime - delay)),
		m_gain{ gain }
	{ }

	void setDelay(float delay) {
		m_readIndex = m_delayBuffer.size() - 44100.f * delay;
	}

	void setGain(float gain) {
		m_gain = gain;
	}

	void write(int16_t sample) {
		m_latestSample = sample;
		m_delayBuffer[m_writeIndex] = sample + m_gain * (m_delayBuffer[m_readIndex] - m_gain * sample);

		m_writeIndex++;
		m_writeIndex %= m_delayBuffer.size();
	}

	int16_t read() {
		return m_delayBuffer[m_readIndex] - m_gain * m_latestSample;
	}

	void step() {
		m_readIndex++;
		m_readIndex %= m_delayBuffer.size();
	}

private:
	float m_gain;
	int16_t m_latestSample = 0;
	std::size_t m_writeIndex = 0;
	std::size_t m_readIndex;
	std::vector<int16_t> m_delayBuffer;
};

void applyReverb(sf::SoundBuffer& sound, float wetness) {
	const sf::Int16* samples = sound.getSamples();
	std::size_t sampleCount = sound.getSampleCount();

	std::vector<sf::Int16> output;

	CombFilter combs[]{
		{1, .03604, .805},
		{1, .03112, .827},
		{1, .04044, .783},
		{1, .04492, .764},
	};

	AllpassFilter allpasses[]{
		{1,   .005, .7},
		{1, .00168, .7},
		{1, .00048, .7},
	};

	output.reserve(sampleCount);

	for (int i = 0; i < sampleCount; i++) {
		auto sample = samples[i];

		sf::Int16 effectedSample = 0;
		for (auto& comb : combs) {
			comb.write(sample);
			effectedSample += comb.read();
			comb.step();
		}
		effectedSample >>= 2;

		for (auto& allpass : allpasses) {
			allpass.write(effectedSample);
			effectedSample = allpass.read();
			allpass.step();
		}

		output.emplace_back(sample * (1 - wetness) + effectedSample * wetness);
	}

	sound.loadFromSamples(output.data(), output.size(), 1, 44100);
}

void applyDelay(sf::SoundBuffer& sound, float wetness) {
	const sf::Int16* samples = sound.getSamples();
	std::size_t sampleCount = sound.getSampleCount();

	std::vector<sf::Int16> output;

	Delay delay(2, 0.2, 0.5);
	for (int i = 0; i < sampleCount; i++) {
		auto sample = samples[i];

		delay.write(sample);

		sample = delay.read();

		delay.step();

		output.emplace_back(sample);
	}

	sound.loadFromSamples(output.data(), output.size(), 1, 44100);
}

void applyBitcrush(sf::SoundBuffer& sound, float wetness) {
	const sf::Int16* samples = sound.getSamples();
	std::size_t sampleCount = sound.getSampleCount();

	std::vector<sf::Int16> output;

	int n = 4;
	for (int i = 0; i < sampleCount; i++) {
		auto sample = samples[i];

		sample >>= 16 - n;
		sample <<= 16 - n;

		output.emplace_back(sample);
	}

	sound.loadFromSamples(output.data(), output.size(), 1, 44100);
}
