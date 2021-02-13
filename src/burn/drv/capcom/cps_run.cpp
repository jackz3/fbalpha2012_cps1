#include<cstdio>
// CPS - Run
#include "cps.h"

// Inputs:
UINT8 CpsReset = 0;
UINT8 Cpi01A = 0, Cpi01C = 0, Cpi01E = 0;

static INT32 nInterrupt;
static INT32 nIrqLine, nIrqCycles;
static bool bEnableAutoIrq50, bEnableAutoIrq52;				// Trigger an interrupt every 32 scanlines

static const INT32 nFirstLine = 0x10;							// The first scanline of the display

static INT32 nCpsCyclesExtra;

INT32 CpsDrawSpritesInReverse = 0;

INT32 nIrqLine50, nIrqLine52;

INT32 nCpsNumScanlines = 259;
INT32 Cps1VBlankIRQLine = 2;

CpsRunInitCallback CpsRunInitCallbackFunction = NULL;
CpsRunInitCallback CpsRunExitCallbackFunction = NULL;
CpsRunResetCallback CpsRunResetCallbackFunction = NULL;
CpsRunFrameStartCallback CpsRunFrameStartCallbackFunction = NULL;
CpsRunFrameMiddleCallback CpsRunFrameMiddleCallbackFunction = NULL;
CpsRunFrameEndCallback CpsRunFrameEndCallbackFunction = NULL;

static void CpsQSoundCheatSearchCallback()
{
	// Q-Sound Shared RAM ranges - not useful for cheat searching, and runs the Z80
	// in the handler, exclude it from cheat searching
	if (Cps1Qs == 1) {	
		CheatSearchExcludeAddressRange(0xF18000, 0xF19FFF);
		CheatSearchExcludeAddressRange(0xF1E000, 0xF1FFFF);
	}
}

static INT32 DrvReset()
{
	// Reset machine
	if (PangEEP || Cps1Qs == 1 || CpsBootlegEEPROM) EEPROMReset();

	SekOpen(0);
	SekReset();
	SekClose();

	if (((Cps & 1) && !Cps1DisablePSnd)) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
	}

	SekOpen(0);
	CpsMapObjectBanks(0);
	SekClose();

	nCpsCyclesExtra = 0;

	if (Cps1Qs == 1) {			// Sound init (QSound)
		QsndReset();
	}
	
	if (CpsRunResetCallbackFunction) {
		CpsRunResetCallbackFunction();
	}
	
	HiscoreReset();

	return 0;
}

static const eeprom_interface qsound_eeprom_interface =
{
	7,		/* address bits */
	8,		/* data bits */
	"0110",	/*  read command */
	"0101",	/* write command */
	"0111",	/* erase command */
	0,
	0,
	0,
	0
};

static const eeprom_interface cps2_eeprom_interface =
{
	6,		/* address bits */
	16,		/* data bits */
	"0110",	/*  read command */
	"0101",	/* write command */
	"0111",	/* erase command */
	0,
	0,
	0,
	0
};

UINT32 cpu_disassemble_i8085(char *buffer, UINT32 pc, const UINT8 *oprom, const UINT8 *opram);
INT32 I8080RunInit() {
	UINT8* oprom = CpsZRom;
	for (int i = 0; i < 26;) {
		char str[100];
		// printf("rom: %d\n", *(oprom + pc));
		UINT32 step = cpu_disassemble_i8085(str, 0, oprom, oprom);
		oprom += step;
		printf("%04x %s\n", i, str);
		i += step;
	}
	printf("disamble ===========\n");
	return 0;
}

INT32 CpsRunInit()
{
	SekInit(0, 0x68000);					// Allocate 68000
	
	if (CpsMemInit()) {						// Memory init
		return 1;
	}
	
	if (PangEEP) {
		EEPROMInit(&cps2_eeprom_interface);
	} else {
		if (Cps1Qs == 1 || CpsBootlegEEPROM) {
			EEPROMInit(&qsound_eeprom_interface);
		}
	}

	CpsRwInit();							// Registers setup

	if (CpsPalInit()) {						// Palette init
		return 1;
	}
	if (CpsObjInit()) {						// Sprite init
		return 1;
	}

	if ((Cps & 1) && Cps1Qs == 0 && Cps1DisablePSnd == 0) {			// Sound init (MSM6295 + YM2151)
		if (PsndInit()) {
			return 1;
		}
	}

	if (Cps1Qs == 1) {			// Sound init (QSound)
		if (QsndInit()) {
			return 1;
		}
		QsndSetRoute(BURN_SND_QSND_OUTPUT_1, 1.00, BURN_SND_ROUTE_LEFT);
		QsndSetRoute(BURN_SND_QSND_OUTPUT_2, 1.00, BURN_SND_ROUTE_RIGHT);
	}

	if (PangEEP || Cps1Qs == 1 || CpsBootlegEEPROM) EEPROMReset();
	
	if (CpsRunInitCallbackFunction) {
		CpsRunInitCallbackFunction();
	}
	
	DrvReset();

	//Init Draw Function
	DrawFnInit();
	
	pBurnDrvPalette = CpsPal;
	
	if (Cps1Qs == 1) {
		CheatSearchInitCallbackFunction = CpsQSoundCheatSearchCallback;
	}

	return 0;
}

INT32 CpsRunExit()
{
	if (PangEEP || Cps1Qs == 1 || CpsBootlegEEPROM) EEPROMExit();

	// Sound exit
	if (Cps1Qs == 1) QsndExit();
	if (Cps != 2 && Cps1Qs == 0 && !Cps1DisablePSnd) PsndExit();

	// Graphics exit
	CpsObjExit();
	CpsPalExit();

	// Sprite Masking exit
	ZBuf = NULL;

	// Memory exit
	CpsRwExit();
	CpsMemExit();

	SekExit();
	
	if (CpsRunExitCallbackFunction) {
		CpsRunExitCallbackFunction();
		CpsRunExitCallbackFunction = NULL;
	}
	CpsRunInitCallbackFunction = NULL;
	CpsRunResetCallbackFunction = NULL;
	CpsRunFrameStartCallbackFunction = NULL;
	CpsRunFrameMiddleCallbackFunction = NULL;
	CpsRunFrameEndCallbackFunction = NULL;
	
	Cps1VBlankIRQLine = 2;
	
	Cps2DisableQSnd = 0;
	CpsBootlegEEPROM = 0;

	return 0;
}

inline static void CopyCpsReg(INT32 i)
{
	memcpy(CpsSaveReg[i], CpsReg, 0x0100);
}

inline static void CopyCpsFrg(INT32 i)
{
	memcpy(CpsSaveFrg[i], CpsFrg, 0x0010);
}

// Schedule a beam-synchronized interrupt
static void ScheduleIRQ()
{
	INT32 nLine = nCpsNumScanlines;

	if (nIrqLine50 <= nLine) {
		nLine = nIrqLine50;
	}
	if (nIrqLine52 < nLine) {
		nLine = nIrqLine52;
	}

	if (nLine < nCpsNumScanlines) {
		nIrqLine = nLine;
		nIrqCycles = (nLine * nCpsCycles / nCpsNumScanlines) + 1;
	} else {
		nIrqCycles = nCpsCycles + 1;
	}

	return;
}

// Execute a beam-synchronised interrupt and schedule the next one
static void DoIRQ()
{
	// 0x4E - bit 9 = 1: Beam Synchronized interrupts disabled
	// 0x50 - Beam synchronized interrupt #1 occurs at raster line.
	// 0x52 - Beam synchronized interrupt #2 occurs at raster line.

	// Trigger IRQ and copy registers.
	if (nIrqLine >= nFirstLine) {

		nInterrupt++;
		nRasterline[nInterrupt] = nIrqLine - nFirstLine;
	}

	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekRun(nCpsCycles * 0x01 / nCpsNumScanlines);
	if (nRasterline[nInterrupt] < 224) {
		CopyCpsReg(nInterrupt);
		CopyCpsFrg(nInterrupt);
	} else {
		nRasterline[nInterrupt] = 0;
	}

	// Schedule next interrupt
	if (!bEnableAutoIrq50) {
		if (nIrqLine >= nIrqLine50) {
			nIrqLine50 = nCpsNumScanlines;
		}
	} else {
		if (bEnableAutoIrq50 && nIrqLine == nIrqLine50) {
			nIrqLine50 += 32;
		}
	}
	if (!bEnableAutoIrq52 && nIrqLine >= nIrqLine52) {
		nIrqLine52 = nCpsNumScanlines;
	} else {
		if (bEnableAutoIrq52 && nIrqLine == nIrqLine52) {
			nIrqLine52 += 32;
		}
	}
	ScheduleIRQ();
	if (nIrqCycles < SekTotalCycles()) {
		nIrqCycles = SekTotalCycles() + 1;
	}

	return;
}

INT32 I8080Frame() {
	printf("frame\n");
	return 1;
}

INT32 Cps1Frame()
{
	INT32 nDisplayEnd, nNext, i;

	if (CpsReset) {
		DrvReset();
	}

	SekNewFrame();
	if (Cps1Qs == 1) {
		QsndNewFrame();
	} else {
		if (!Cps1DisablePSnd) {
			ZetOpen(0);
			PsndNewFrame();
		}
	}
	
	if (CpsRunFrameStartCallbackFunction) {
		CpsRunFrameStartCallbackFunction();
	}

	nCpsCycles = (INT32)((INT64)nCPS68KClockspeed * nBurnCPUSpeedAdjust >> 8);

	CpsRwGetInp();												// Update the input port values

	nDisplayEnd = (nCpsCycles * (nFirstLine + 224)) / nCpsNumScanlines;	// Account for VBlank

	SekOpen(0);
	SekIdle(nCpsCyclesExtra);

	SekRun(nCpsCycles * nFirstLine / nCpsNumScanlines);					// run 68K for the first few lines

	CpsObjGet();											// Get objects

	for (i = 0; i < 4; i++) {
		nNext = ((i + 1) * nCpsCycles) >> 2;					// find out next cycle count to run to
		
		if (i == 2 && CpsRunFrameMiddleCallbackFunction) {
			CpsRunFrameMiddleCallbackFunction();
		}

		if (SekTotalCycles() < nDisplayEnd && nNext > nDisplayEnd) {

			SekRun(nNext - nDisplayEnd);						// run 68K

			memcpy(CpsSaveReg[0], CpsReg, 0x100);				// Registers correct now

			SekSetIRQLine(Cps1VBlankIRQLine, SEK_IRQSTATUS_AUTO);				// Trigger VBlank interrupt
		}

		SekRun(nNext - SekTotalCycles());						// run 68K
		
//		if (pBurnDraw) {
//			CpsDraw();										// Draw frame
//		}
	}

	if (!nSkipFrame) {
		CpsDraw();										// Draw frame
	}

	if (Cps1Qs == 1) {
		QsndEndFrame();
	} else {
		if (!Cps1DisablePSnd) {
			PsndSyncZ80(nCpsZ80Cycles);
			PsmUpdate(nBurnSoundLen);
			ZetClose();
		}
	}
	
	if (CpsRunFrameEndCallbackFunction) {
		CpsRunFrameEndCallbackFunction();
	}

	nCpsCyclesExtra = SekTotalCycles() - nCpsCycles;

	SekClose();

	return 0;
}
