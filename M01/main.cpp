//cpp-attempt on https://mavenanalytics.io/data-drills/turning-bullish
#include "../common/TxtFile.h"
#include "../common/SplitIter.h"
#include "../common/StopWatch.h"
#include <print>

#define NOMINMAX
#include <Windows.h> //GetModuleFilename, needed to locate Data-Folder relative to executable


#include <ranges>
#include <iomanip>
#include <map>
#include <array>
#include <numeric>
#include <chrono>
#include <bitset>
#include <thread>
using ymd = std::chrono::year_month_day;
enum class DataAt : int {
	Date,
	Close,
	SMA50,
	SMA200,
	AlarmsUp,
	AlarmsDown,
};

using Data = std::tuple<
	ymd,
	double,
	double,
	double,
	std::bitset<1>, //bullishflags
	std::bitset<1> //bearishflags
>;

constexpr double makeDouble(auto&& str) {
	double d{};
	std::from_chars(str.data(), str.data() + str.size(), d,std::chars_format::fixed); 
	return d;
}

constexpr std::optional<ymd> makeDate(auto&& dateStr) //auto&& because of std::subrange<<>,<>..>, it is no string_view, but splittable on char
{
	using namespace std::chrono;
	short y{};
	unsigned int m{};
	unsigned int d{};
	for (int idx{}; auto item : dateStr | std::views::split('-'))
	{
		++idx;
		switch (idx)
		{
		case(1):
		{
			auto [ptr, ec] = std::from_chars(item.data(), item.data() + item.size(), y);
			if (ec != std::errc{}) {
				return {};
			}
		}
		break;
		case(2):
		{
			auto [ptr, ec] = std::from_chars(item.data(), item.data() + item.size(), m);
			if (ec != std::errc{}) {
				return {};
			}
		}
		break;
		case(3):
		{
			auto [ptr, ec] = std::from_chars(item.data(), item.data() + item.size(), d);
			if (ec != std::errc{}) {
				return {};
			}
		}
		break;
		default:
			break;
		}
	}
	return ymd{ year{y},month{m},day{d} };
}


template<int stride, DataAt source, DataAt target>
void sma(std::vector<Data>& data) {
	if (stride <= 0) { return; }
	if (data.size() < stride) { return; }
	constexpr int sourceIdx = static_cast<int>(source);
	constexpr int targetIdx = static_cast<int>(target);
	
	double lastsum{};
	for (int pos = 0; pos < stride; ++pos) {
		lastsum += std::get<sourceIdx>(data[pos]);
	}
	std::get<targetIdx>(data[stride - 1]) = lastsum / stride;

	for (int pos = stride; pos < data.size(); ++pos) {
		lastsum -= std::get<sourceIdx>(data[pos - stride]);;
		lastsum += std::get<sourceIdx>(data[pos]);
		std::get<targetIdx>(data[pos]) = lastsum / stride;
	}
}

template<DataAt shouldCrossingUp, DataAt shouldBeCrossed, 
	DataAt target=DataAt::AlarmsUp, size_t flagPos=0ull
>
void mark_cross_up( std::vector<Data>& data_){
	constexpr int i1 = static_cast<int>(shouldCrossingUp);
	constexpr int i2 = static_cast<int>(shouldBeCrossed);
	constexpr int targetIdx = static_cast<int>(target);

	for (size_t i = 1; i < data_.size(); ++i) {
		const Data& prev = data_[i - 1];
		Data& current = data_[i];
		const double prevDelta = std::get<i2>(prev) - std::get<i1>(prev);
		if (prevDelta < 0.0) { 
			continue; 
		}
		const double currDelta = std::get<i2>(current) - std::get<i1>(current);
		if (currDelta < 0.0) {
			auto& flags = std::get<targetIdx>(current);
			flags.set(flagPos, true);
		}
	}
}

void run(const fs::path& inputFile)
{
	TxtFile txt(inputFile);
	size_t linecount = std::count(txt.buf.begin(), txt.buf.end(), '\n');
	
	std::vector<Data> points{ }; //start
	points.reserve(linecount);

	for (int idx{}; const auto& line : txt) {
		++idx;
		if (idx == 1) [[unlikely]] { continue; } //skip head

		auto items = line | std::views::split(',');

		Data pt{};
		for (int idx{}; auto&& val : items) {
			switch (idx) {
			case(0): std::get<static_cast<int>(DataAt::Date)>(pt) = makeDate(val).value_or(ymd{}); break;
			case(1): std::get<static_cast<int>(DataAt::Close)>(pt) = makeDouble(val); break;
			}
			++idx;
		}
		if (pt != Data{} && std::get<0>(pt) != ymd{} ) {
			points.emplace_back(pt);
		}
	}

	sma<50, DataAt::Close, DataAt::SMA50>(points);
	sma<200, DataAt::Close, DataAt::SMA200>(points);
	
	mark_cross_up<DataAt::SMA50, DataAt::SMA200, DataAt::AlarmsUp, 0ull>(points);
	mark_cross_up<DataAt::SMA200, DataAt::SMA50, DataAt::AlarmsDown, 0ull>(points);
	
	double crossAt{};
	for (const Data& item : points) {
		const auto& flagsUp = std::get < static_cast<int>(DataAt::AlarmsUp)>(item);
		if (flagsUp.test(0ull)) {
			std::println("50 crossing 200 up:   {} {:.2f}", std::get<0>(item), std::get<1>(item) );
			crossAt = std::get<1>(item);
		}
		const auto& flagsDown = std::get < static_cast<int>(DataAt::AlarmsDown)>(item);
		if (flagsDown.test(0ull)) {
			std::println("50 crossing 200 down: {} {:.2f}", std::get<0>(item), std::get<1>(item) );
		}
	}
	std::println("\npart: lastup {:g}", crossAt);
}

fs::path FindInput() {
	char pFile[MAX_PATH]{};
	::GetModuleFileName(NULL, pFile, MAX_PATH);
	//src/bin/debug/modulefile.exe
	//src/bin/release/modulefile.exe
	//src/inputs.txt
	fs::path inputFile = fs::path(pFile).parent_path().parent_path().parent_path() / "SPY_close_price_5Y.csv";
	if (!fs::exists(inputFile))
	{
		std::println("input not found: {}", inputFile.string());
		return {};
	}
	return inputFile;
}


int main(int argc, const char* argv[])
{
	StopWatch timeIt(std::cout);
	SetConsoleOutputCP(1252); //latin1

	auto inputFile = FindInput();
	run(inputFile);
	return 0;
}