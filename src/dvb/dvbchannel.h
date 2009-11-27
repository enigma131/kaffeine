/*
 * dvbchannel.h
 *
 * Copyright (C) 2007-2009 Christoph Pfister <christophpfister@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DVBCHANNEL_H
#define DVBCHANNEL_H

#include <QSharedData>
#include <QString>

class AtscTransponder;
class DvbCTransponder;
class DvbSTransponder;
class DvbS2Transponder;
class DvbTTransponder;
class DvbTransponder;

class DvbTransponderBase : public QSharedData
{
public:
	enum TransmissionType {
		DvbC = 0,
		DvbS = 1,
		DvbS2 = 4,
		DvbT = 2,
		Atsc = 3
	};

	enum FecRate {
		FecNone = 0,
		Fec1_2 = 1,
		Fec1_3 = 10,
		Fec1_4 = 11,
		Fec2_3 = 2,
		Fec2_5 = 12,
		Fec3_4 = 3,
		Fec3_5 = 13,
		Fec4_5 = 4,
		Fec5_6 = 5,
		Fec6_7 = 6,
		Fec7_8 = 7,
		Fec8_9 = 8,
		Fec9_10 = 14,
		FecAuto = 9,
		FecRateMax = Fec9_10
	};

	DvbTransponderBase() { }
	virtual ~DvbTransponderBase() { }

	virtual TransmissionType getTransmissionType() const = 0;

	virtual const DvbCTransponder *getDvbCTransponder() const
	{
		return NULL;
	}

	virtual const DvbSTransponder *getDvbSTransponder() const
	{
		return NULL;
	}

	virtual const DvbS2Transponder *getDvbS2Transponder() const
	{
		return NULL;
	}

	virtual const DvbTTransponder *getDvbTTransponder() const
	{
		return NULL;
	}

	virtual const AtscTransponder *getAtscTransponder() const
	{
		return NULL;
	}

	virtual void readTransponder(QDataStream &stream) = 0;
	virtual void writeTransponder(QDataStream &stream) const = 0;

	/*
	 * convert from / to linuxtv scan file format
	 */

	virtual bool fromString(const QString &string) = 0;
	virtual QString toString() const = 0;

	/*
	 * corresponding in this context means that both tuning parameters will lead to the same
	 * transponder; note the tuning parameters don't have to be equal, it's sufficient that they
	 * can't coexist at the same time (for example the frequency difference between two channels
	 * in the same network has to be big enough because of bandwith)
	 */

	virtual bool corresponds(const DvbTransponder &transponder) const = 0;

	QString source;
};

class DvbCTransponder : public DvbTransponderBase
{
public:
	enum Modulation {
		Qam16 = 0,
		Qam32 = 1,
		Qam64 = 2,
		Qam128 = 3,
		Qam256 = 4,
		ModulationAuto = 5,
		ModulationMax = ModulationAuto
	};

	TransmissionType getTransmissionType() const
	{
		return DvbC;
	}

	const DvbCTransponder *getDvbCTransponder() const
	{
		return this;
	}

	void readTransponder(QDataStream &stream);
	void writeTransponder(QDataStream &stream) const;
	bool fromString(const QString &string);
	QString toString() const;

	bool corresponds(const DvbTransponder &transponder) const;

	int frequency; // Hz
	int symbolRate; // symbols per second
	Modulation modulation;
	FecRate fecRate;
};

class DvbSTransponder : public DvbTransponderBase
{
public:
	enum Polarization {
		Horizontal = 0,
		Vertical = 1,
		CircularLeft = 2,
		CircularRight = 3,
		PolarizationMax = CircularRight
	};

	TransmissionType getTransmissionType() const
	{
		return DvbS;
	}

	const DvbSTransponder *getDvbSTransponder() const
	{
		return this;
	}

	void readTransponder(QDataStream &stream);
	void writeTransponder(QDataStream &stream) const;
	bool fromString(const QString &string);
	QString toString() const;

	bool corresponds(const DvbTransponder &transponder) const;

	Polarization polarization;
	int frequency; // kHz
	int symbolRate; // symbols per second
	FecRate fecRate;
};

class DvbS2Transponder : public DvbSTransponder
{
public:
	enum Modulation {
		Qpsk = 0,
		Psk8 = 1,
		Apsk16 = 2,
		Apsk32 = 3,
		ModulationAuto = 4,
		ModulationMax = ModulationAuto
	};

	enum RollOff {
		RollOff20 = 0,
		RollOff25 = 1,
		RollOff35 = 2,
		RollOffAuto = 3,
		RollOffMax = RollOffAuto
	};

	TransmissionType getTransmissionType() const
	{
		return DvbS2;
	}

	const DvbSTransponder *getDvbSTransponder() const
	{
		return NULL;
	}

	const DvbS2Transponder *getDvbS2Transponder() const
	{
		return this;
	}

	void readTransponder(QDataStream &stream);
	void writeTransponder(QDataStream &stream) const;
	bool fromString(const QString &string);
	QString toString() const;

	bool corresponds(const DvbTransponder &transponder) const;

	Modulation modulation;
	RollOff rollOff;
};

class DvbTTransponder : public DvbTransponderBase
{
public:
	enum Bandwidth {
		Bandwidth6MHz = 0,
		Bandwidth7MHz = 1,
		Bandwidth8MHz = 2,
		BandwidthAuto = 3,
		BandwidthMax = BandwidthAuto
	};

	enum Modulation {
		Qpsk = 0,
		Qam16 = 1,
		Qam64 = 2,
		ModulationAuto = 3,
		ModulationMax = ModulationAuto
	};

	enum TransmissionMode {
		TransmissionMode2k = 0,
		TransmissionMode8k = 1,
		TransmissionModeAuto = 2,
		TransmissionModeMax = TransmissionModeAuto
	};

	enum GuardInterval {
		GuardInterval1_4 = 0,
		GuardInterval1_8 = 1,
		GuardInterval1_16 = 2,
		GuardInterval1_32 = 3,
		GuardIntervalAuto = 4,
		GuardIntervalMax = GuardIntervalAuto
	};

	enum Hierarchy {
		HierarchyNone = 0,
		Hierarchy1 = 1,
		Hierarchy2 = 2,
		Hierarchy4 = 3,
		HierarchyAuto = 4,
		HierarchyMax = HierarchyAuto
	};

	TransmissionType getTransmissionType() const
	{
		return DvbT;
	}

	const DvbTTransponder *getDvbTTransponder() const
	{
		return this;
	}

	void readTransponder(QDataStream &stream);
	void writeTransponder(QDataStream &stream) const;
	bool fromString(const QString &string);
	QString toString() const;

	bool corresponds(const DvbTransponder &transponder) const;

	int frequency; // Hz
	Bandwidth bandwidth;
	Modulation modulation;
	FecRate fecRateHigh; // high priority stream
	FecRate fecRateLow; // low priority stream
	TransmissionMode transmissionMode;
	GuardInterval guardInterval;
	Hierarchy hierarchy;
};

class AtscTransponder : public DvbTransponderBase
{
public:
	enum Modulation {
		Qam64 = 0,
		Qam256 = 1,
		Vsb8 = 2,
		Vsb16 = 3,
		ModulationAuto = 4,
		ModulationMax = ModulationAuto
	};

	TransmissionType getTransmissionType() const
	{
		return Atsc;
	}

	const AtscTransponder *getAtscTransponder() const
	{
		return this;
	}

	void readTransponder(QDataStream &stream);
	void writeTransponder(QDataStream &stream) const;
	bool fromString(const QString &string);
	QString toString() const;

	bool corresponds(const DvbTransponder &transponder) const;

	int frequency; // Hz
	Modulation modulation;
};

class DvbTransponder : public QExplicitlySharedDataPointer<const DvbTransponderBase>
{
public:
	explicit DvbTransponder(const DvbTransponderBase *transponder = NULL) :
		QExplicitlySharedDataPointer<const DvbTransponderBase>(transponder) { }
	~DvbTransponder() { }
};

class DvbChannelBase
{
public:
	DvbChannelBase() : number(-1), networkId(-1), transportStreamId(-1), pmtPid(-1),
		audioPid(-1), hasVideo(false), isScrambled(false) { }
	~DvbChannelBase() { }

	void readChannel(QDataStream &stream);

	int getServiceId() const
	{
		if (pmtSection.size() < 5) {
			return -1;
		}

		return ((static_cast<unsigned char>(pmtSection.at(3)) << 8) |
			static_cast<unsigned char>(pmtSection.at(4)));
	}

	void setServiceId(int serviceId)
	{
		if (pmtSection.size() < 5) {
			return;
		}

		pmtSection[3] = (serviceId >> 8);
		pmtSection[4] = (serviceId & 0xff);
	}

	QString name;
	int number;

	DvbTransponder transponder;
	int networkId; // may be -1 (not present); ATSC meaning: source id
	int transportStreamId;
	int pmtPid;

	QByteArray pmtSection;
	int audioPid; // may be -1 (not present)
	bool hasVideo;
	bool isScrambled;
};

class DvbChannel : public DvbChannelBase, public QSharedData
{
public:
	DvbChannel() { }
	explicit DvbChannel(const DvbChannelBase &channel) : DvbChannelBase(channel) { }
	~DvbChannel() { }
};

#endif /* DVBCHANNEL_H */
