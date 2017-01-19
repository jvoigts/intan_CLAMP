#include "SaveFile.h"
#include "Board.h"
#include "common.h"
#include "Constants.h"
#include "streams.h"
#include <ctime>

#ifdef WIN32
#pragma warning(disable: 4996) // On Windows, disable the warning about localtime being bad.
#endif

using std::wstring;
using std::unique_ptr;
using namespace CLAMP;
using namespace CLAMP::ClampConfig;
using std::vector;
using std::invalid_argument;
using std::time_t;

namespace CLAMP {
    namespace IO {
        // Save files are buffered in memory, i.e., data is written to a big block in memory, then to disk.  This is the
        // size of that big block of memory.
        static const unsigned int SAVE_BUFFER_SIZE = 4 * CLAMP::KILO;

        // Saved data file constants
		#define DATA_FILE_MAGIC_NUMBER  0xf3b1a481
		#define DATA_FILE_MAIN_VERSION_NUMBER  1
		#define DATA_FILE_SECONDARY_VERSION_NUMBER  0

        //------------------------------------------------------------------------------------------------------
        /// Constructor
        TimeDate::TimeDate() {
            // Initialize to now.

            time_t t = time(0);
            struct tm* now = localtime(&t);
            year = now->tm_year + 1900;
            month = now->tm_mon + 1;
            day = now->tm_mday;
            hour = now->tm_hour;
            minute = now->tm_min;
            second = now->tm_sec;
        }

        /** Size of this object on disk
         *  \returns The size in bytes
         */
        unsigned int TimeDate::onDiskSize() const {
            return 6 * sizeof(int16_t);
        }

        /** \brief Write the timestamp to a BinaryWriter stream
         *
         * \param[in] out   Output stream
         * \param[in] timestamp  Timestamp to write
         * \returns Updated output stream
         */
        BinaryWriter& operator<<(BinaryWriter& out, const TimeDate& timestamp) {
            out << (int16_t)timestamp.year;
            out << (int16_t)timestamp.month;
            out << (int16_t)timestamp.day;
            out << (int16_t)timestamp.hour;
            out << (int16_t)timestamp.minute;
            out << (int16_t)timestamp.second;

            return out;
        }

        //------------------------------------------------------------------------------------------------------
        /** \brief Constructor
         *
         *  \param[in] board  Used for register settings, calibration settings, etc.
         *  \param[in] index  Which ChipChannel the settings correspond to
         */
        VoltageClampSettings::VoltageClampSettings(Board& board, const ChipChannel& index)
        {
            Channel& channel = board.controller.getChannel(index);

            desiredBandwidth = channel.desiredBandwidth;
            actualBandwidth = channel.getActualBandwidth();
            nominalResistance = channel.getNominalResistance();
            resistance = channel.getFeedbackResistance();
        }

        /** Size of this object on disk
         *  \returns The size in bytes
         */
        unsigned int VoltageClampSettings::onDiskSize() const {
            return 5 * sizeof(float);
        }

        /** \brief Write the settings to a BinaryWriter stream
         *
         * \param[in] out   Output stream
         * \param[in] settings  Settings to write
         * \returns Updated output stream
         */
        BinaryWriter& operator<<(BinaryWriter& out, const VoltageClampSettings& settings) {
            out << settings.holdingVoltage;

            out << settings.nominalResistance;
            out << settings.resistance;
            out << settings.desiredBandwidth;
            out << settings.actualBandwidth;

            return out;
        }

        //------------------------------------------------------------------------------------------------------
        /** Size of this object on disk
         *  \returns The size in bytes
         */
        unsigned int CurrentClampSettings::onDiskSize() const {
            return 2 * sizeof(float);
        }

        /** \brief Write the settings to a BinaryWriter stream
         *
         * \param[in] out   Output stream
         * \param[in] settings  Settings to write
         * \returns Updated output stream
         */
        BinaryWriter& operator<<(BinaryWriter& out, const CurrentClampSettings& settings) {
            out << settings.holdingCurrent;
            out << settings.stepSize;

            return out;
        }

        //------------------------------------------------------------------------------------------------------
        /** \brief Constructor
         *
         *  \param[in] board  Used for register settings, calibration settings, etc.
         *  \param[in] index  Which ChipChannel the settings correspond to
         */
        Settings::Settings(Board& board, const ChipChannel& index) :
            voltageClamp(board, index)
        {
            ClampController& controller = board.controller;

            enableCapacitiveCompensation = controller.fastTransientCapacitiveCompensation.getConnect(index);
            capCompensationMagnitude = controller.fastTransientCapacitiveCompensation.getMagnitude(index);

            samplingRate = board.getSamplingRateHz();
        }

        /** Size of this object on disk
         *  \returns The size in bytes
         */
        unsigned int Settings::onDiskSize() const {
            unsigned int commonSize = sizeof(uint8_t) + 7 * sizeof(float) + 2 * sizeof(uint8_t);
            unsigned int clampSize = isVoltageClamp ? voltageClamp.onDiskSize() : currentClamp.onDiskSize();
            return commonSize + clampSize + waveform.onDiskSize();
        }

        /** \brief Write the settings to a BinaryWriter stream
         *
         * \param[in] out   Output stream
         * \param[in] settings  Settings to write
         * \returns Updated output stream
         */
        BinaryWriter& operator<<(BinaryWriter& out, const Settings& settings) {
            out << (uint8_t)settings.enableCapacitiveCompensation;
            out << settings.capCompensationMagnitude;
            out << settings.filterCutoff;
            out << settings.pipetteOffset;
            out << settings.samplingRate;
			out << settings.Ra;
			out << settings.Rm;
			out << settings.Cm;
            out << (uint8_t)settings.isVoltageClamp;
			out << (uint8_t)settings.vClampX2mode;
            if (settings.isVoltageClamp) {
                out << settings.voltageClamp;
            }
            else {
                out << settings.currentClamp;
            }
            out << settings.waveform;

            return out;
        }

        //------------------------------------------------------------------------------------------------------
        /** \brief Constructor
        *
        *  \param[in] b      Board.  Used for register settings, calibration settings, etc.
        *  \param[in] index  Which ChipChannel the settings correspond to
        */
        HeaderData::HeaderData(Board& b, const ChipChannel& index) :
            board(b),
            version(DATA_FILE_MAIN_VERSION_NUMBER, DATA_FILE_SECONDARY_VERSION_NUMBER),
            settings(b, index)
        {
        }

        /** \brief Write the header to a BinaryWriter stream
         *
         * \param[in] out   Output stream
         * \param[in] header  Header to write
         * \returns Updated output stream
         */
        BinaryWriter& operator<<(BinaryWriter& out, const HeaderData& header) {
            // Save file and version information.
            out << (uint32_t)DATA_FILE_MAGIC_NUMBER;
            out << header.version.majorVersion;
            out << header.version.minorVersion;
			out << (uint16_t)0; // 0 = headstage header (not aux)

            uint16_t headerSizeBytes = sizeof(uint32_t)+sizeof(header.version.majorVersion) + sizeof(header.version.minorVersion) + 2*sizeof(uint16_t) /* signature and this field */
                + header.timestamp.onDiskSize()
                + 2 * sizeof(uint16_t) + MAX_NUM_CHIPS * (MAX_NUM_CHANNELS * header.board.chip[0]->channel[0]->onDiskSize() + 4 * sizeof(uint16_t)) /* chip and channel data */
                + header.settings.onDiskSize() /* Settings */;

			int timestampSize = header.timestamp.onDiskSize();
			int channelSize = header.board.chip[0]->channel[0]->onDiskSize();
			int settingsSize = header.settings.onDiskSize();


            out << headerSizeBytes;

            out << header.timestamp;

            // Now write chip/channel settings
            out << (uint16_t)MAX_NUM_CHIPS;
            out << (uint16_t)MAX_NUM_CHANNELS;
            for (unsigned int chipIndex = 0; chipIndex < MAX_NUM_CHIPS; chipIndex++) {
                const Chip& chip = *header.board.chip[chipIndex];

                for (unsigned int channelIndex = 0; channelIndex < MAX_NUM_CHANNELS; channelIndex++) {
                    const Channel& channel = *chip.channel[channelIndex];
                    out << channel;
                }

                for (unsigned int registerIndex = 0; registerIndex <= 3; registerIndex++) {
                    uint16_t registerValue = const_cast<Chip&>(chip).chipRegisters.get(registerIndex).value;
                    out << registerValue;
                }
            }

            out << header.settings;

            return out;
        }

		//------------------------------------------------------------------------------------------------------
		/** \brief Constructor
		*
		*  \param[in] b      Board.  Used for register settings, calibration settings, etc.
		*/
		AuxHeaderData::AuxHeaderData(Board& b, const ChipChannel& index, int numAdcs_) :
			numAdcs(numAdcs_),
			version(DATA_FILE_MAIN_VERSION_NUMBER, DATA_FILE_SECONDARY_VERSION_NUMBER),
			settings(b, index)
		{
		}

		/** \brief Write the header to a BinaryWriter stream
		*
		* \param[in] out   Output stream
		* \param[in] header  Header to write
		* \returns Updated output stream
		*/
		BinaryWriter& operator<<(BinaryWriter& out, const AuxHeaderData& header) {
			// Save file and version information.
			out << (uint32_t)DATA_FILE_MAGIC_NUMBER;
			out << header.version.majorVersion;
			out << header.version.minorVersion;
			out << (uint16_t)1; // 1 = aux data header
			out << (uint16_t)header.numAdcs;

			uint16_t headerSizeBytes = sizeof(uint32_t) + sizeof(header.version.majorVersion) + sizeof(header.version.minorVersion) + 3*sizeof(uint16_t) /* signature and this field */
				+ header.timestamp.onDiskSize() + sizeof(float);

			out << headerSizeBytes;

			out << header.timestamp;

			out << header.settings.samplingRate;

			return out;
		}

        //------------------------------------------------------------------------------------------------------
        SaveFile::SaveFile()
        {

        }

        SaveFile::~SaveFile()
        {
            close();
        }

        /** \brief Open a file for saving.
         *
         *  \param[in] path  Path of the file
         */
        void SaveFile::open(const FILENAME& path) {
            unique_ptr<FileOutStream> fs(new FileOutStream());
            fs->open(path);

            unique_ptr<BinaryWriter> bs(new BinaryWriter(std::move(fs), SAVE_BUFFER_SIZE));
            file.reset(bs.release());
        }

        /// Close the save file
        void SaveFile::close() {
            file.reset(nullptr);
        }

        /** \brief Write the header of the save file.
         *
         *  This function should only be called once per save file, and that should be after open and before any
         *  writeData() calls.
         *
         * \param[in] header   The header data to write
         */
        void SaveFile::writeHeader(HeaderData& header) {
            *file << header;

            waveform = header.settings.waveform;
        }

		void SaveFile::writeHeaderAux(AuxHeaderData& auxHeader) {
			*file << auxHeader;
		}

        /** \brief Write a block of data to the file.
         *
         *  If you're looping over the same waveform multiple times, you should call this once per time you loop over it.  The
         *  waveform length is stored in the header, and code reading files will look for this structure to match that.
         *
         * \param[in] timestamps    Timestamps
         * \param[in] measuredData  Measured current in voltage clamp mode or measured voltage in current clamp mode
         * \param[in] adcs          Values of the ADCs
         */
        void SaveFile::writeData(const vector<uint32_t>& timestamps, const vector<double>& measuredData, const vector<double>& clampValues) {
            bool sizesMatch = (timestamps.size() == measuredData.size());
            if (!sizesMatch) {
                throw invalid_argument("Size mismatch");
            }

            vector<double> applied = waveform.getApplied(timestamps);

            unsigned int size = timestamps.size();
            for (unsigned int i = 0; i < size; i++) {
                *file << timestamps[i];
                *file << applied[i];
				*file << clampValues[i];
                *file << measuredData[i];
            }
        }

		void SaveFile::writeDataAux(const vector<uint32_t>& timestamps, const vector<vector<uint16_t>>& adcs, int numAdcs, const vector<uint16_t>& digIns, const vector<uint16_t>& digOuts) {
			bool sizesMatch = (adcs[0].size() == timestamps.size());  // If we were being really pedantic, we'd do a loop and check adcs[i] for all i

			unsigned int size = timestamps.size();
			for (unsigned int i = 0; i < size; i++) {
				*file << timestamps[i];
				*file << digIns[i];
				*file << digOuts[i];
				for (int adc = 0; adc < numAdcs; adc++) {
					*file << adcs[adc][i];
				}
			}
		}
    }
}
