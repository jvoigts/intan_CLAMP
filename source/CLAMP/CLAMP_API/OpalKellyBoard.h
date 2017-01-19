#pragma once

#include <memory>
#include <vector>
#include "OpalKellyLibraryHandle.h"
#include <cstdint>
#include <mutex>

/** \brief This class provides access to and control of the Opal Kelly XEM6010 USB/FPGA interface board running the Clamp interface Verilog code.
	*
	*  Only one instance of the OpalKellyBoard object is needed to control an FPGA interface.
	*
	*  The functions in this class typically throw exceptions if an error occurs while talking to the FPGA board.  Calling code should be
	*  aware of that and wrap calls in try/catch as necessary.  Individual functions aren't necessarily documented as throwing exceptions; just be aware
	*  that all of the communications functions can fail and will throw in that case.
	*
	*  See also https://library.opalkelly.com/library/FrontPanelAPI/index.html for more information on the underlying library functions.
*/
class OpalKellyBoard {
public:
	OpalKellyBoard() {}
	virtual ~OpalKellyBoard();

	virtual void loadLibrary(okFP_dll_pchar dllPath);
	virtual bool open(const std::string& dllPath = "", const std::string& bitfilePath = "", const std::string& requestedSerialNumber = "");
	virtual void uploadFpgaBitfile(const std::string& filename);

	// Wires In
	virtual void updateWiresIn();
	virtual void setWireIn(int wirein, uint16_t value);
	virtual void setWireInBit(int wirein, unsigned long bit, bool value);

	// Triggers In
	virtual void activateTriggerIn(int epAddr, int bit);

	// Pipes In
	virtual long writeToPipeIn(int epAddr, long length, unsigned char *data);

	// Wires Out
	virtual void updateWiresOut();
	virtual bool getWireOutBit(int wireout, unsigned long bit);
	virtual uint16_t getWireOutWord(int wireout);
	virtual uint32_t getWireOutDWord(int wireoutMSB, int wireoutLSB);

	// Pipes Out
	virtual long readFromPipeOut(int epAddr, long length, unsigned char *data);

	virtual bool isOpen() const;

private:
	// Functions in this class are designed to be thread-safe.  This variable is used to ensure that.
	std::mutex ioMutex;

	std::unique_ptr<OpalKellyLibraryHandle> library;
	std::unique_ptr<okCFrontPanel> frontPanel;

	std::vector<std::string> getSerialNumbers();
	double getSystemClockFreq() const;

	static std::string opalKellyModelName(int model);
	static void checkError(okCFrontPanel::ErrorCode code);
};

