#ifndef SPIFFSCHECKSUM_HPP
#define SPIFFSCHECKSUM_HPP

struct {
	const char* name;
	uint32_t sum;
} static const SPIFFSChecksums[] = {
	{ "/spiffs/battery/shutdown.raw", 272088},
	{ "/spiffs/drive/arrow-down-active.raw", 6707},
	{ "/spiffs/drive/arrow-down.raw", 11487},
	{ "/spiffs/drive/arrow-left-active.raw", 6707},
	{ "/spiffs/drive/arrow-left.raw", 11487},
	{ "/spiffs/drive/arrow-right-active.raw", 6707},
	{ "/spiffs/drive/arrow-right.raw", 11487},
	{ "/spiffs/drive/arrow-up-active.raw", 6707},
	{ "/spiffs/drive/arrow-up.raw", 11487},
	{ "/spiffs/drive/connected.raw", 237300},
	{ "/spiffs/drive/mute.raw", 29427},
	{ "/spiffs/drive/panicBar.gif", 48779},
	{ "/spiffs/drive/panicText.raw", 86352},
	{ "/spiffs/drive/quality-bar.raw", 132084},
	{ "/spiffs/drive/quality-line.raw", 3726},
	{ "/spiffs/drive/signal1.raw", 18462},
	{ "/spiffs/drive/signal2.raw", 20822},
	{ "/spiffs/drive/signal3.raw", 24952},
	{ "/spiffs/drive/signal4.raw", 30852},
	{ "/spiffs/drive/signal5.raw", 17282},
	{ "/spiffs/intro/cross.raw", 10782},
	{ "/spiffs/intro/logo-cm.raw", 3089004},
	{ "/spiffs/intro/logo-geek.raw", 1796652},
	{ "/spiffs/intro/logo-perse.raw", 979301},
	{ "/spiffs/intro/logo-space.raw", 627126},
	{ "/spiffs/pair/button.gif", 217195},
	{ "/spiffs/pair/controller_rover.raw", 659260},
	{ "/spiffs/pair/error.raw", 331652},
	{ "/spiffs/pair/frame.raw", 502656},
	{ "/spiffs/pair/signal.gif", 38794},
	{ "/spiffs/pair/textDC.raw", 247806},
	{ "/spiffs/pair/textFail.raw", 264024},
	{ "/spiffs/pair/textPair.raw", 282537},
};

#endif //SPIFFSCHECKSUM_HPP
