#include <iostream>
#include <map>
#include <list>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <thread>
#include <chrono>

#include "../Engine/Audio/audio.h"
#include "../Engine/Utils/TextFileParser.h"
#include "../Engine/Logger/logger.h"
#include "../Engine/Math/ComplexMath.h"
#include "../Engine/Assets/package.h"

float Wave(float x)
{
	x = sin(x);
	return x;
}

std::vector<float> Flute(float frequency, size_t duration)
{
	std::vector<float> buffer(duration * 2, 0);

	float step = 0.001;
	size_t edgeLen = 1.0 / step;
	float multiplier = 0;

	for (size_t index = 0; index < duration; ++index) {
		if (duration - index < edgeLen) {
			multiplier -= step;
		} else if (multiplier < 1.0) {
			multiplier += step;
		}

		float value = sin(
			(float)index / Audio::SampleRate * 2 * M_PI *
			frequency) * multiplier;

		buffer[index * 2] = value;
		buffer[index * 2 + 1] = value;
	}

	return buffer;
}

std::vector<float> Piano(float frequency, size_t duration)
{
	std::vector<float> buffer(duration * 2, 0);

	float step = 0.0005;
	size_t edgeLen = 1.0 / step;
	float multiplier = 0;

	float forceDec = 0.000002;

	for (size_t tone = 1; tone <= 8; ++tone) {
		float force = 1;

		for (size_t index = 0; index < duration; ++index) {
			if (duration - index < edgeLen) {
				multiplier -= step;
			} else if (multiplier < 1.0) {
				multiplier += step;
			}

			float waveValue = Wave(
				(float)index / Audio::SampleRate * 2 * M_PI *
				frequency * tone);

			float addMul = 1.0f +
				0.5 * std::max<int64_t>(
					(float)(Audio::SampleRate * 0.5 -
						(int64_t)index) /
					(Audio::SampleRate * 0.5),
					0);

			float value =
				waveValue * addMul *
				multiplier * force / powf(2, tone);

			buffer[index * 2] += value;
			buffer[index * 2 + 1] += value;

			force -= forceDec * powf(frequency / 120.0, 1);

			if (force < 0) {
				force = 0;
			}
		}
	}

	return buffer;
}

std::vector<float> FilterSound(std::vector<float> buffer)
{
	std::vector<float> result(buffer.size(), 0);

	result[0] = buffer[0];
	result[1] = buffer[1];
	result[buffer.size() - 1] = buffer[buffer.size() - 1];
	result[buffer.size() - 2] = buffer[buffer.size() - 2];

	for (size_t index = 2; index < result.size() - 2; index += 2) {
		result[index] = (buffer[index - 2] + buffer[index + 2]) / 2;
		result[index + 1] = (buffer[index - 1] + buffer[index + 3]) / 2;
	}

	return result;
}

std::vector<float> NormalizeSound(std::vector<float> buffer)
{
	std::vector<float> result(buffer.size(), 0);

	float maxValue = *std::max_element(buffer.begin(), buffer.end());

	for (size_t i = 0; i < buffer.size(); ++i) {
		result[i] = buffer[i] / maxValue;
	}

	return result;
}

void cb(size_t idx, size_t lim)
{
	Logger::Verbose() << idx << " / " << lim;
}

std::vector<float> DFTConvert(std::vector<float> buffer, int shift)
{
	size_t len = buffer.size() / 2;

	std::vector<Complex> complexBuffer(len);

	for (size_t i = 0; i < len; ++i) {
		complexBuffer[i] = buffer[i * 2];
	}

	Logger::Verbose() << "DFT ...";
	std::vector<Complex> dftBuffer = DFT(complexBuffer);
	std::vector<Complex> shiftBuffer(len);

	size_t low = std::max<int>(0, -shift);
	size_t high = std::min<int>(len, len - shift);

	for (size_t i = low; i < high; ++i) {
		shiftBuffer[i] = dftBuffer[i + shift];
	}

	Logger::Verbose() << "IDFT ...";
	complexBuffer = IDFT(shiftBuffer);

	std::vector<float> result(buffer.size(), 0);

	for (size_t i = 0; i < len; ++i) {
		float value = complexBuffer[i].Mod();

		result[i * 2] = value;
		result[i * 2 + 1] = value;
	}

	return result;
}

std::vector<float> CompileFile(std::string filename)
{
	TextFileParser::File file = TextFileParser::ParseFile(filename);

	std::map<std::string, float> frequencies;

	struct Note
	{
		std::string Name;
		float Multiplier;
		float Start;
		float Duration;
	};

	struct Loop
	{
		size_t Count;
		float Start;
		float Delay;
	};

	std::list<Note> notes;
	std::list<Loop> loops;

	loops.push_back({1, 0, 0});

	float baseFrequency = 1000;

	float duration = 0;

	float timeBase = 0;

	bool openEnding = false;

	for (auto& line : file) {
		if (line.size() == 0) {
			continue;
		}

		if (line[0][0] == '#') {
			continue;
		}

		if (line[0] == "define") {
			std::string name = line[1];
			float frequency = std::stof(line[2]);

			frequencies[name] = frequency;
		} else if (line[0] == "setbase") {
			timeBase += std::stof(line[1]);
		} else if (line[0] == "loop") {
			Loop loop;
			loop.Count = std::stoi(line[1]);
			loop.Start = std::stof(line[2]);
			loop.Delay = std::stof(line[3]);

			loops.push_back(loop);
		} else if (line[0] == "endloop") {
			loops.pop_back();
		} else if (line[0] == "note") {
			for (size_t i = 0; i < loops.back().Count; ++i) {
				Note note;
				note.Name = line[1];
				note.Multiplier = std::stof(line[2]);
				note.Start = std::stof(line[3]) + timeBase +
					loops.back().Delay * i +
					loops.back().Start;

				if (openEnding) {
					notes.back().Duration = 0.1 +
						note.Start - notes.back().Start;
					openEnding = false;
				}

				if (line.size() >= 5) {
					note.Duration = std::stof(line[4]);
				} else {
					openEnding = true;
				}

				float end = note.Start + note.Duration;
				if (end > duration) {
					duration = end;
				}

				notes.push_back(note);
			}
		} else {
			throw std::runtime_error("Invalid command.");
		}
	}

	if (openEnding) {
		notes.back().Duration = 20;
	}

	std::vector<float> buffer(duration * Audio::SampleRate * 2, 0);

	for (auto& note : notes) {
		Logger::Verbose() << "Note: " << note.Name;
		size_t lim = note.Duration * Audio::SampleRate;
		size_t start = note.Start * Audio::SampleRate;

		if (lim + start > buffer.size() / 2) {
			lim -= lim + start - buffer.size() / 2;
		}

		size_t pos = start;

		std::vector<float> sound = Piano(
			frequencies[note.Name],
			lim);

		//sound = DFTConvert(sound, 0);

		float ampCoeff = baseFrequency / frequencies[note.Name];

		for (size_t index = 0; index < lim; ++index) {

			buffer[pos * 2] += sound[index * 2] *
				ampCoeff * note.Multiplier;
			buffer[pos * 2 + 1] += sound[index * 2 + 1] *
				ampCoeff * note.Multiplier;

			++pos;
		}
	}

	buffer = NormalizeSound(buffer);
	buffer = FilterSound(buffer);
	buffer = NormalizeSound(buffer);

	return buffer;
}

std::vector<float> GenerateFrequencies(int octCount, int noteCount)
{
	std::vector<float> frequencies(octCount * noteCount);

	float o1LaFreq = 440;

	float noteStep = powf(2.0, 1.0 / (double)noteCount);
	float octStep = 2.0;

	size_t index = 0;

	for (int oct = 0; oct < octCount; ++oct) {
		for (int note = -5; note <= 1; ++note) {
			float value = o1LaFreq * powf(octStep, oct) *
				powf(noteStep, note);

			frequencies[index] = value;
			++index;
		}
	}

	return frequencies;
}

std::vector<float> ProcessTimeNotation(std::string filename)
{
	int octCount = 6;
	int noteCount = 7;
	float nps = 1;

	int notes = octCount * noteCount;

	std::vector<bool> noteState(notes, false);
	std::vector<float> frequencies = GenerateFrequencies(
		octCount,
		noteCount);
	std::vector<float> starts(notes, 0.0);

	struct Slot
	{
		float Frequency;
		size_t Start;
		size_t Length;
	};

	std::list<Slot> slots;

	TextFileParser::File file = TextFileParser::ParseFile(filename, {});
	float currentTime = 0.0;

	for (auto& line : file) {
		std::string word = line[0];

		if (word[0] == ' ' || word[0] == 'f') {
			// Process note line.
			std::vector<bool> curNoteState(notes, false);

			int index = 0;
			for (char c : word) {
				if (c == 'f') {
					curNoteState[index] = true;
				}

				++index;
			}

			for (int index = 0; index < notes; ++index) {
				if (noteState[index] && !curNoteState[index]) {
					Slot slot;
					slot.Frequency = frequencies[index];
					slot.Start = starts[index] *
						Audio::SampleRate;
					slot.Length = currentTime *
						Audio::SampleRate - slot.Start;

					slots.push_back(slot);
				}

				if (!noteState[index] && curNoteState[index]) {
					starts[index] = currentTime;
				}

				noteState[index] = curNoteState[index];
			}

			currentTime += 1.0 / nps;
		} else {
			// Process command.
			if (word.starts_with("nps")) {
				nps = std::stof(word.substr(4));
			} else if (word.starts_with("npm")) {
				nps = std::stof(word.substr(4)) / 60.0;
			}
		}
	}

	for (int index = 0; index < notes; ++index) {
		if (noteState[index]) {
			Slot slot;
			slot.Frequency = frequencies[index];
			slot.Start = starts[index] * Audio::SampleRate;
			slot.Length = currentTime * Audio::SampleRate -
				slot.Start;

			slots.push_back(slot);
		}
	}

	std::vector<float> buffer(
		(currentTime + 1) * Audio::SampleRate * 2,
		0.0);

	for (auto& slot : slots) {
		std::vector<float> sound = Flute(slot.Frequency, slot.Length);

		for (size_t i = 0; i < slot.Length * 2; ++i) {
			buffer[i + slot.Start * 2] += sound[i];
		}
	}

	buffer = FilterSound(buffer);
	buffer = NormalizeSound(buffer);

	return buffer;
}

int main(int argc, char** argv)
{
	Logger::SetLevel(Logger::Level::Verbose);

	Package::LoadPackage("resources.bin");

	Audio audio;
	Audio::Buffer buffer;

	Logger::Verbose() << "Compiling.";

	if (strcmp(argv[1], "-t")) {
		buffer.Data = CompileFile(argv[1]);
	} else {
		buffer.Data = ProcessTimeNotation(argv[2]);
	}

	Logger::Verbose() << "Playing.";
	audio.Submit(&buffer);

	std::this_thread::sleep_for(
		std::chrono::seconds(
			buffer.Data.size() / Audio::SampleRate / 2 + 1));

	Package::UnloadPackage();

	return 0;
}
