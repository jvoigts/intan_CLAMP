#pragma once

#include <string>
#include <memory>
#include <vector>

#include "streams.h"
#include "ClampController.h"
#include "SimplifiedWaveform.h"

class BinaryWriter;
namespace CLAMP {
    class Board;

    /** \brief Save file format and related.
     *
     *  See SaveFile for details.
     */
    namespace IO {
        /// A date/time timestamp
        struct TimeDate {
            /// Year
            int year;
            /// Month [1-12]
            int month;
            /// Day of month [1-31]
            int day;
            /// Hour of the day [0-23]
            int hour;
            /// Minute [0-60]
            int minute;
            /// Second [0-60]
            int second;

            TimeDate();

            unsigned int onDiskSize() const;

            friend BinaryWriter& operator<<(BinaryWriter& out, const TimeDate& timestamp);
        };

        /** \brief Voltage Clamp settings, for inclusion in SaveFile
         *
         *  Note that this does not include the waveform itself, which is stored as a SimplifiedWaveform.
         *
         *  Only used if Settings::isVoltageClamp is true.
         */
        struct VoltageClampSettings {
            /// Holding voltage (i.e., voltage before and after waveform)
            double holdingVoltage;
            /// Nominal value (in &Omega;) of the feedback resistor used.  See also \ref resistance.
            double nominalResistance;
            /// Measured value (in &Omega;) of the feedback resistor used.  See also \ref nominalResistance.
            double  resistance;
            /// Desired cutoff frequency of on-chip low pass filters
            double  desiredBandwidth;
            /// Actual (i.e., best achievable) cutoff frequency of on-chip low pass filters
            double  actualBandwidth;

            VoltageClampSettings(CLAMP::Board& board, const CLAMP::ClampConfig::ChipChannel& index);

            unsigned int onDiskSize() const;
            friend BinaryWriter& operator<<(BinaryWriter& out, const VoltageClampSettings& settings);
        };

        /** \brief Current Clamp settings, for inclusion in SaveFile
         *
         *  Note that this does not include the waveform itself, which is stored as a SimplifiedWaveform.
         *
         *  Only used if Settings::isVoltageClamp is false.
         */
        struct CurrentClampSettings {
            /// Holding current (i.e., current before and after waveform)
            double holdingCurrent;
            /// Size of current step, in Amperes (i.e., which scale of current generation is used).
            double stepSize;

            unsigned int onDiskSize() const;
            friend BinaryWriter& operator<<(BinaryWriter& out, const CurrentClampSettings& settings);
        };

        /** \brief Settings for inclusion in SaveFile.
         *
         *  The save file also contains the raw register values, but some of the settings are easier
         *  to read out in this form.
         */
        struct Settings {
            /// True if fast transient capacitive compensation is enabled
            bool enableCapacitiveCompensation;
            /// Magnitude of fast transient capacitive compensation, in pF
            double capCompensationMagnitude;

            /// True for voltage clamp mode, false for current clamp mode
            bool isVoltageClamp;
			/// True for 2x voltage clamp mode (5 mV steps instead of 2.5 mV steps)
			bool vClampX2mode;
            /// If isVoltageClamp=true, contains voltage clamp settings.
            VoltageClampSettings voltageClamp;
            /// If isVoltageClamp=false, contains current clamp settings.
            CurrentClampSettings currentClamp;

            /// Filter cutoff frequency, in Hz.  0 means 'no filtering.'
            double filterCutoff;
            /// Pipette voltage offset, in volts
            double pipetteOffset;

            /// Sampling rate, in Hz
            double samplingRate;

			/// Last measured values of cell parameters Rm, Cm, and Ra
			double Ra;
			double Rm;
			double Cm;

            /// Waveform that was applied
            CLAMP::SimplifiedWaveform waveform;

            Settings(CLAMP::Board& board, const CLAMP::ClampConfig::ChipChannel& index);

            unsigned int onDiskSize() const;
            friend BinaryWriter& operator<<(BinaryWriter& out, const Settings& settings);
        };

        /// A version number (e.g., 1.3)
        struct Version {
            /// Major version number (e.g., 1 in 1.3)
            uint16_t majorVersion;
            /// Minor version number (e.g., 3 in 1.3)
            uint16_t minorVersion;

            /** Constructor
             *
             * \param[in] ma   Major version number
             * \param[in] mi   Minor version number
             */
            Version(uint16_t ma, uint16_t mi) : majorVersion(ma), minorVersion(mi) {}
        };
        /// True if version a < version b
        bool operator<(const Version& a, const Version& b);
        /// True if version a >= version b
        bool operator>=(const Version& a, const Version& b);


        /// Data that is stored in the header of a save file
        struct HeaderData {
        private:
            CLAMP::Board& board;

        public:
            /// Version
            Version version;

            /// Time stamp when the data file was started
            TimeDate timestamp;
            /// Settings that control the waveform that was run
            Settings settings;

            HeaderData(CLAMP::Board& b, const CLAMP::ClampConfig::ChipChannel& index);

            friend BinaryWriter& operator<<(BinaryWriter& out, const HeaderData& header);
        };

		/// Data that is stored in the header of an auxiliary save file (ADCs and digital I/O)
		struct AuxHeaderData {
		private:

		public:
			/// Version
			Version version;

			/// Number of ADCs
			int numAdcs;

			/// Time stamp when the data file was started
			TimeDate timestamp;

			Settings settings;

			AuxHeaderData(CLAMP::Board& b, const CLAMP::ClampConfig::ChipChannel& index, int numAdcs_);

			friend BinaryWriter& operator<<(BinaryWriter& out, const AuxHeaderData& header);
		};

        /** \brief A CLAMP save file
         *
         *  Right now, we only have a single file format.  If the software is modified to support multiple formats,
         *  this class should become an abstract base class, the members should become virtual, and the subclasses
         *  should implement open(), close(), writeHeader(), and writeData() in their own formats.
         */
        class SaveFile {
        public:
            SaveFile();
            ~SaveFile();
            void open(const FILENAME& path);
            void close();
            void writeHeader(HeaderData& header);
			void writeHeaderAux(AuxHeaderData& auxHeader);
            void writeData(const std::vector<uint32_t>& timestamps, const std::vector<double>& measuredData, const std::vector<double>& clampValues);
			void writeDataAux(const std::vector<uint32_t>& timestamps, const std::vector<std::vector<uint16_t>>& adcs, int numAdcs, const std::vector<uint16_t>& digIns, const std::vector<uint16_t>& digOuts);

        private:
            std::unique_ptr<BinaryWriter> file;
            CLAMP::SimplifiedWaveform waveform;
        };
    }
}

