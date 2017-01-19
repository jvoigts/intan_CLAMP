#include "OpalKellyBoard.h"
#include "OpalKellyLibraryHandle.h"
#include "Constants.h"
#include "common.h"
#include <exception>
#include <sstream>

using std::string;
using std::endl;
using std::vector;
using std::unique_ptr;
using std::runtime_error;
using std::ostringstream;
using std::invalid_argument;
using std::lock_guard;
using std::mutex;
using namespace CLAMP;


OpalKellyBoard::~OpalKellyBoard() {
	frontPanel.reset();
	library.reset();
}

/** \brief Loads the Opal Kelly DLL from a non-default location.

	You don't need to call this if you just want to use the library in its default location; in that case, open() will
	load the library for you.

	@param[in] dllPath	Path of the okFrontPanel dll.
*/
void OpalKellyBoard::loadLibrary(okFP_dll_pchar dllPath) {
	if (library.get() == nullptr) {
		library.reset(OpalKellyLibraryHandle::create(dllPath));
	}
}

vector<string> OpalKellyBoard::getSerialNumbers() {
	vector<string> results;

	LOG(logOpalKelly) << endl << "Scanning USB for Opal Kelly devices..." << endl << endl;

	int nDevices = frontPanel->GetDeviceCount();
	LOG(logOpalKelly) << "Found " << nDevices << " Opal Kelly device" << ((nDevices == 1) ? "" : "s") << " connected:" << endl;

	// Log devices, and store devices in list of type XEM6010LX45.
	for (int i = 0; i < nDevices; ++i) {
		LOG(logOpalKelly) << "  Device #" << i + 1 << ": Opal Kelly " << opalKellyModelName(frontPanel->GetDeviceListModel(i)).c_str() <<
			" with serial number " << frontPanel->GetDeviceListSerial(i).c_str() << endl;

		if (frontPanel->GetDeviceListModel(i) == OK_PRODUCT_XEM6010LX45) {
			results.push_back(frontPanel->GetDeviceListSerial(i));
		}
	}
	LOG(logOpalKelly) << endl;

	return results;
}

// Return name of Opal Kelly board based on model code.
string OpalKellyBoard::opalKellyModelName(int model)
{
	switch (model) {
	case OK_PRODUCT_XEM3001V1:
		return("XEM3001V1");
	case OK_PRODUCT_XEM3001V2:
		return("XEM3001V2");
	case OK_PRODUCT_XEM3010:
		return("XEM3010");
	case OK_PRODUCT_XEM3005:
		return("XEM3005");
	case OK_PRODUCT_XEM3001CL:
		return("XEM3001CL");
	case OK_PRODUCT_XEM3020:
		return("XEM3020");
	case OK_PRODUCT_XEM3050:
		return("XEM3050");
	case OK_PRODUCT_XEM9002:
		return("XEM9002");
	case OK_PRODUCT_XEM3001RB:
		return("XEM3001RB");
	case OK_PRODUCT_XEM5010:
		return("XEM5010");
	case OK_PRODUCT_XEM6110LX45:
		return("XEM6110LX45");
	case OK_PRODUCT_XEM6001:
		return("XEM6001");
	case OK_PRODUCT_XEM6010LX45:
		return("XEM6010LX45");
	case OK_PRODUCT_XEM6010LX150:
		return("XEM6010LX150");
	case OK_PRODUCT_XEM6110LX150:
		return("XEM6110LX150");
	case OK_PRODUCT_XEM6006LX9:
		return("XEM6006LX9");
	case OK_PRODUCT_XEM6006LX16:
		return("XEM6006LX16");
	case OK_PRODUCT_XEM6006LX25:
		return("XEM6006LX25");
	case OK_PRODUCT_XEM5010LX110:
		return("XEM5010LX110");
	case OK_PRODUCT_ZEM4310:
		return("ZEM4310");
	case OK_PRODUCT_XEM6310LX45:
		return("XEM6310LX45");
	case OK_PRODUCT_XEM6310LX150:
		return("XEM6310LX150");
	case OK_PRODUCT_XEM6110V2LX45:
		return("XEM6110V2LX45");
	case OK_PRODUCT_XEM6110V2LX150:
		return("XEM6110V2LX150");
	case OK_PRODUCT_XEM6002LX9:
		return("XEM6002LX9");
	case OK_PRODUCT_XEM6310MTLX45:
		return("XEM6310MTLX45");
	case OK_PRODUCT_XEM6320LX130T:
		return("XEM6320LX130T");
	default:
		return("UNKNOWN");
	}
}

/** \brief Finds an Opal Kelly XEM6010 - LX45 board attached to a USB port and opens it.

	@param[in] dllPath	Path of the okFrontPanel dll.
	@param[in] bitfilePath   Path of FPGA bitfile.
	@param[in] requestedSerialNumber   Serial number of the board to attach to.  If you pass "", the
										function will pick the first available board.
										
*/
bool OpalKellyBoard::open(const string& dllPath, const string& bitfilePath, const string& requestedSerialNumber) {
	frontPanel.reset(nullptr);

	LOG(logOpalKelly) << "---- Intan Technologies ---- CLAMP Controller v1.0 ----" << endl << endl;

	// If library isn't already loaded, do so (from default location)
	loadLibrary(nullptr);  // TODO: add dllPath

	frontPanel.reset(new okCFrontPanel);

	vector<string> serialNumbers;
	string serialNumber = requestedSerialNumber;
	if (serialNumber != "") {
		serialNumbers.push_back(serialNumber);
	}
	else {
		serialNumbers = getSerialNumbers();
	}

	if (!serialNumbers.empty()) {
		for (unsigned int i = 0; i < serialNumbers.size(); i++) {
			LOG(logOpalKelly) << "Trying board " << i << "..." << endl;
			serialNumber = serialNumbers[i];
			if (frontPanel->OpenBySerial(serialNumber) == okCFrontPanel::NoError) {
				// Configure the on-board PLL appropriately.
				frontPanel->LoadDefaultPLLConfiguration();

				// Get some general information about the XEM.
				LOG(logOpalKelly) << "FPGA system clock: " << getSystemClockFreq() << " MHz" << endl; // Should indicate 100 MHz
				LOG(logOpalKelly) << "Opal Kelly device firmware version: " << frontPanel->GetDeviceMajorVersion() << "." << frontPanel->GetDeviceMinorVersion() << endl;
				LOG(logOpalKelly) << "Opal Kelly device serial number: " << frontPanel->GetSerialNumber().c_str() << endl;
				LOG(logOpalKelly) << "Opal Kelly device ID string: " << frontPanel->GetDeviceID().c_str() << endl << endl;

				uploadFpgaBitfile(bitfilePath == "" ? "main.bit" : bitfilePath);

				updateWiresOut();
				int boardMode = getWireOutWord(WireOut::BoardMode);
				LOG(logOpalKelly) << "Board mode = " << boardMode << endl;

				if (boardMode != CLAMP_BOARD_MODE) continue;

				int boardId = getWireOutWord(WireOut::BoardId);
				int boardVersion = getWireOutWord(WireOut::BoardVersion);

				if (boardId != CLAMP_BOARD_ID) continue;

				LOG(logOpalKelly) << "Clamp configuration file successfully loaded.  Clamp version number: " << boardVersion << endl << endl;

				return true;
			}
		}
	}

	frontPanel.reset(nullptr);
	return false;
}


// Reads system clock frequency from Opal Kelly board (in MHz).  Should be 100 MHz for normal
// Rhythm operation.
double OpalKellyBoard::getSystemClockFreq() const
{
	// Read back the CY22393 PLL configuation
	okCPLL22393 pll;
	frontPanel->GetEepromPLL22393Configuration(pll);

	return pll.GetOutputFrequency(0);
}

void OpalKellyBoard::checkError(okCFrontPanel::ErrorCode code) {
	switch (code) {
	case okCFrontPanel::NoError:
		return;
	case okCFrontPanel::DeviceNotOpen:
		throw runtime_error("FPGA error: Device not open.");
	case okCFrontPanel::FileError:
		throw runtime_error("FPGA error: Cannot find configuration file.");
	case okCFrontPanel::InvalidBitstream:
		throw runtime_error("FPGA error: Bitstream is not properly formatted.");
	case okCFrontPanel::DoneNotHigh:
		throw runtime_error("FPGA error: FPGA DONE signal did not assert after configuration.");
	case okCFrontPanel::TransferError:
		throw runtime_error("FPGA error: USB error occurred during download.");
	case okCFrontPanel::CommunicationError:
		throw runtime_error("FPGA error: Communication error with firmware.");
	case okCFrontPanel::UnsupportedFeature:
		throw runtime_error("FPGA error: Unsupported feature.");
	case okCFrontPanel::Timeout:
		throw runtime_error("FPGA error: Timeout.");
	default: {
		ostringstream tmp;
		tmp << "FPGA error: Unknown error: " << code << ".";
		throw runtime_error(tmp.str().c_str());
	}
	}
}

/** \brief Uploads the bitfile to the Xilinx FPGA on the open Opal Kelly board.

	@param[in] filename Path of the configuration file.  Either relative to the executable file or absolute path.
*/
void OpalKellyBoard::uploadFpgaBitfile(const string& filename)
{
	okCFrontPanel::ErrorCode errorCode = frontPanel->ConfigureFPGA(filename);
	checkError(errorCode);

	// Check for Opal Kelly FrontPanel support in the FPGA configuration.
	if (frontPanel->IsFrontPanelEnabled() == false) {
		frontPanel.reset();
		throw runtime_error("Opal Kelly FrontPanel support is not enabled in this FPGA configuration.");
	}
}

/** \brief Call frontPanel's UpdateWireIns.
*/
void OpalKellyBoard::updateWiresIn() {
	lock_guard<mutex> lockio(ioMutex);

	frontPanel->UpdateWireIns();
}

/** \brief Sets the given wire in to the given value.  Does not call UpdateWireIns.

	Throws an exception on error.

	@param[in] wirein   WireIn address
	@param[in] value    16-bit value to set it to
*/
void OpalKellyBoard::setWireIn(int wirein, uint16_t value) {
	checkError(frontPanel->SetWireInValue(wirein, value));
}

/** \brief Sets the given wire in's bit to the given value.  Does not call UpdateWireIns.

	Throws an exception on error.

	@param[in] wirein   WireIn address
	@param[in] bit      Mask of which bit it is
	@param[in] value    Whether the bit should be set or cleared
*/
void OpalKellyBoard::setWireInBit(int wirein, unsigned long bit, bool value) {
	checkError(frontPanel->SetWireInValue(wirein, value ? bit : 0, bit));
}

/** \brief Call frontPanel's ActivateTriggerIn.

	Throws an exception on error.

	@param[in] epAddr   TriggerIn address
	@param[in] bit      Mask of which bit it is
*/
void OpalKellyBoard::activateTriggerIn(int epAddr, int bit) {
	lock_guard<mutex> lockio(ioMutex);
	checkError(frontPanel->ActivateTriggerIn(epAddr, bit));
}

/** \brief Call frontPanel's UpdateWireOuts.
*/
void OpalKellyBoard::updateWiresOut() {
	lock_guard<mutex> lockio(ioMutex);
	frontPanel->UpdateWireOuts();
}

/** \brief Gets the given wire out-bit.

	@param[in] wireout  WireOut address
	@param[in] bit      Mask of which bit it is

	@return  True if the bit is set.
*/
bool OpalKellyBoard::getWireOutBit(int wireout, unsigned long bit) {
	unsigned long word = getWireOutWord(wireout);
	return !!(word & bit);
}

/** \brief Gets the given WireOut word value (2 bytes).  Does not call UpdateWireOuts first.

	@param[in] wireout   WireOut address

	@return  value
*/
uint16_t OpalKellyBoard::getWireOutWord(int wireout) {
	return static_cast<uint16_t>(frontPanel->GetWireOutValue(wireout) & 0xFFFF);
}

/** \brief Gets the given WireOut double word value (4 bytes).  Does not call UpdateWireOuts first.

	@param[in] wireoutMSB   WireOut address of the most significant bits
	@param[in] wireoutLSB   WireOut address of the least significant bits

	@return  value
*/
uint32_t OpalKellyBoard::getWireOutDWord(int wireoutMSB, int wireoutLSB) {
	return (getWireOutWord(wireoutMSB) << 16) + getWireOutWord(wireoutLSB);
}

/** \brief Call frontPanel's ReadFromPipeOut.

	Throws an exception on error.

	@param[in] epAddr   PipeOut address
	@param[in] length   Number of bytes to read
	@param[in] data     Buffer to put the data into
	@returns The number of bytes read (may be different than length if not enough data was available).
*/
long OpalKellyBoard::readFromPipeOut(int epAddr, long length, unsigned char *data) {
	if (length % 8 != 0) {
		throw invalid_argument("Read length must be divisible by 8.");
	}

	lock_guard<mutex> lockio(ioMutex);
	long ret = frontPanel->ReadFromPipeOut(epAddr, length, data);
	if (ret < 0) {
		checkError(static_cast<okCFrontPanel::ErrorCode>(ret));
	}
	return ret;
}

/** \brief Call frontPanel's WriteToPipeIn.

	Throws an exception on error.

	@param[in] epAddr   PipeIn address
	@param[in] length   Number of bytes to write
	@param[in] data     Data to write
	@returns The number of bytes written.
*/
long OpalKellyBoard::writeToPipeIn(int epAddr, long length, unsigned char *data) {
	lock_guard<mutex> lockio(ioMutex);
	long ret = frontPanel->WriteToPipeIn(epAddr, length, data);
	if (ret < 0) {
		checkError(static_cast<okCFrontPanel::ErrorCode>(ret));
	}
	return ret;
}

/** \brief Checks if the board has been successfully opened.
	*
	*  \returns True if the board has been successfully opened.
	*/
bool OpalKellyBoard::isOpen() const {
	return frontPanel.get() != nullptr;
}
