/*
 * dvbconfigdialog.cpp
 *
 * Copyright (C) 2007-2008 Christoph Pfister <christophpfister@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "dvbconfigdialog.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KTabWidget>
#include "dvbdevice.h"
#include "dvbmanager.h"

DvbConfigDialog::DvbConfigDialog(QWidget *parent, DvbManager *manager_) : KDialog(parent),
	manager(manager_)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setCaption(i18n("DVB Settings"));

	KTabWidget *tabWidget = new KTabWidget(this);
	setMainWidget(tabWidget);

	// FIXME general options

	int i = 1;

	foreach (const DvbDeviceConfig &deviceConfig, manager->getDeviceConfigs()) {
		DvbConfigPage *configPage = new DvbConfigPage(tabWidget, manager, deviceConfig);
		tabWidget->addTab(configPage, KIcon("video-television"), i18n("Device %1", i));
		configPages.append(configPage);
		++i;
	}

	connect(this, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
}

DvbConfigDialog::~DvbConfigDialog()
{
}

void DvbConfigDialog::dialogAccepted()
{
	QList<QList<DvbConfig> > deviceConfigs;

	foreach (DvbConfigPage *configPage, configPages) {
		deviceConfigs.append(configPage->getConfigs());
	}

	manager->setDeviceConfigs(deviceConfigs);
}

DvbConfigPage::DvbConfigPage(QWidget *parent, DvbManager *manager,
	const DvbDeviceConfig &deviceConfig) : QWidget(parent), dvbSObject(NULL)
{
	QBoxLayout *boxLayout = new QVBoxLayout(this);
	boxLayout->addWidget(new QLabel(i18n("Name: %1", deviceConfig.frontendName)));

	DvbDevice *device = deviceConfig.device;

	if (device == NULL) {
		QFrame *frame = new QFrame(this);
		frame->setFrameShape(QFrame::HLine);
		boxLayout->addWidget(frame);

		boxLayout->addWidget(new QLabel(i18n("device not connected")));

		// FIXME option to remove device config

		boxLayout->addStretch();
		return;
	}

	DvbConfig dvbCConfig;
	QList<DvbConfig> dvbSConfigs;
	DvbConfig dvbTConfig;
	DvbConfig atscConfig;

	foreach (const DvbConfig &config, deviceConfig.configs) {
		switch (config->getTransmissionType()) {
		case DvbConfigBase::DvbC:
			dvbCConfig = config;
			break;
		case DvbConfigBase::DvbS:
			dvbSConfigs.append(config);
			break;
		case DvbConfigBase::DvbT:
			dvbTConfig = config;
			break;
		case DvbConfigBase::Atsc:
			atscConfig = config;
			break;
		}
	}

	DvbDevice::TransmissionTypes transmissionTypes = device->getTransmissionTypes();

	if ((transmissionTypes & DvbDevice::DvbC) != 0) {
		DvbConfigBase *config;

		if (dvbCConfig.constData() != NULL) {
			config = new DvbConfigBase(*dvbCConfig);
		} else {
			config = new DvbConfigBase(DvbConfigBase::DvbC);
			config->timeout = 1500;
		}

		configs.append(DvbConfig(config));
		new DvbConfigObject(this, boxLayout, manager, config);
	}

	if ((transmissionTypes & DvbDevice::DvbS) != 0) {
		dvbSObject = new DvbSConfigObject(this, boxLayout, manager, dvbSConfigs);
	}

	if ((transmissionTypes & DvbDevice::DvbT) != 0) {
		DvbConfigBase *config;

		if (dvbTConfig.constData() != NULL) {
			config = new DvbConfigBase(*dvbTConfig);
		} else {
			config = new DvbConfigBase(DvbConfigBase::DvbT);
			config->timeout = 1500;
		}

		configs.append(DvbConfig(config));
		new DvbConfigObject(this, boxLayout, manager, config);
	}

	if ((transmissionTypes & DvbDevice::Atsc) != 0) {
		DvbConfigBase *config;

		if (atscConfig.constData() != NULL) {
			config = new DvbConfigBase(*atscConfig);
		} else {
			config = new DvbConfigBase(DvbConfigBase::Atsc);
			config->timeout = 1500;
		}

		configs.append(DvbConfig(config));
		new DvbConfigObject(this, boxLayout, manager, config);
	}

	boxLayout->addStretch();
}

DvbConfigPage::~DvbConfigPage()
{
}

QList<DvbConfig> DvbConfigPage::getConfigs()
{
	if (dvbSObject != NULL) {
		dvbSObject->appendConfigs(configs);
	}

	for (int i = 0; i < configs.count(); ++i) {
		const DvbConfig &config = configs.at(i);

		if (config->name.isEmpty() || config->scanSource.isEmpty()) {
			configs.removeAt(i);
			--i;
		}
	}

	return configs;
}

DvbConfigObject::DvbConfigObject(QWidget *parent, QBoxLayout *layout, DvbManager *manager,
	DvbConfigBase *config_) : QObject(parent), config(config_)
{
	QFrame *frame = new QFrame(parent);
	frame->setFrameShape(QFrame::HLine);
	layout->addWidget(frame);

	QStringList sources;

	switch (config->getTransmissionType()) {
	case DvbConfigBase::DvbC:
		defaultName = i18n("Cable");
		sources = manager->getScanSources(DvbManager::DvbC);
		layout->addWidget(new QLabel(i18n("DVB-C")));
		break;
	case DvbConfigBase::DvbS:
		Q_ASSERT(false);
		break;
	case DvbConfigBase::DvbT:
		defaultName = i18n("Terrestrial");
		sources = manager->getScanSources(DvbManager::DvbT);
		layout->addWidget(new QLabel(i18n("DVB-T")));
		break;
	case DvbConfigBase::Atsc:
		defaultName = i18n("Atsc");
		sources = manager->getScanSources(DvbManager::Atsc);
		layout->addWidget(new QLabel(i18n("ATSC")));
		break;
	}

	QGridLayout *gridLayout = new QGridLayout();
	layout->addLayout(gridLayout);

	gridLayout->addWidget(new QLabel(i18n("Tuner timeout (ms):")), 0, 0);

	QSpinBox *spinBox = new QSpinBox(parent);
	spinBox->setRange(100, 5000);
	spinBox->setSingleStep(100);
	spinBox->setValue(config->timeout);
	connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(timeoutChanged(int)));
	gridLayout->addWidget(spinBox, 0, 1);

	gridLayout->addWidget(new QLabel(i18n("Source:")), 1, 0);

	sourceBox = new KComboBox(parent);
	sourceBox->addItem(i18n("No source"));
	sourceBox->addItems(sources);
	sourceBox->setCurrentIndex(sources.indexOf(config->scanSource) + 1);
	connect(sourceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceChanged(int)));
	gridLayout->addWidget(sourceBox, 1, 1);

	gridLayout->addWidget(new QLabel(i18n("Name:")), 2, 0);

	nameEdit = new KLineEdit(parent);
	nameEdit->setText(config->name);
	connect(nameEdit, SIGNAL(editingFinished()), this, SLOT(nameChanged()));
	gridLayout->addWidget(nameEdit, 2, 1);

	timeoutChanged(spinBox->value());
	sourceChanged(sourceBox->currentIndex());
	nameChanged();
}

DvbConfigObject::~DvbConfigObject()
{
}

void DvbConfigObject::timeoutChanged(int timeout)
{
	config->timeout = timeout;
}

void DvbConfigObject::sourceChanged(int index)
{
	if (index <= 0) {
		// no source selected
		nameEdit->setEnabled(false);
		config->scanSource.clear();
	} else {
		nameEdit->setEnabled(true);
		config->scanSource = sourceBox->currentText();
	}
}

void DvbConfigObject::nameChanged()
{
	QString name = nameEdit->text();

	if (name.isEmpty()) {
		nameEdit->setText(defaultName);
		config->name = defaultName;
	} else {
		config->name = name;
	}
}

DvbSConfigObject::DvbSConfigObject(QWidget *parent_, QBoxLayout *boxLayout, DvbManager *manager,
	const QList<DvbConfig> &configs) : QObject(parent_), parent(parent_)
{
	if (!configs.isEmpty()) {
		lnbConfig = new DvbConfigBase(*configs.at(0));
	} else {
		lnbConfig = createConfig(0);
	}

	sources = manager->getScanSources(DvbManager::DvbS);

	QFrame *frame = new QFrame(parent);
	frame->setFrameShape(QFrame::HLine);
	boxLayout->addWidget(frame);

	boxLayout->addWidget(new QLabel(i18n("DVB-S")));

	layout = new QGridLayout();
	boxLayout->addLayout(layout);

	layout->addWidget(new QLabel(i18n("Tuner timeout (ms):")), 0, 0);

	QSpinBox *spinBox = new QSpinBox(parent);
	spinBox->setRange(100, 5000);
	spinBox->setSingleStep(100);
	spinBox->setValue(lnbConfig->timeout);
	layout->addWidget(spinBox, 0, 1);

	layout->addWidget(new QLabel(i18n("Configuration:")), 1, 0);

	configBox = new KComboBox(parent);
	configBox->addItem(i18n("DiSEqC switch"));
	configBox->addItem(i18n("USALS rotor"));
	configBox->addItem(i18n("Positions rotor"));
	configBox->setCurrentIndex(lnbConfig->configuration);
	connect(configBox, SIGNAL(currentIndexChanged(int)), this, SLOT(configChanged(int)));
	layout->addWidget(configBox, 1, 1);

	// Diseqc switch

	for (int lnbNumber = 0; lnbNumber < 4; ++lnbNumber) {
		DvbConfigBase *config;

		if ((lnbConfig->configuration == DvbConfigBase::DiseqcSwitch) &&
		    (lnbNumber < configs.size())) {
			config = new DvbConfigBase(*configs.at(lnbNumber));
		} else {
			config = createConfig(lnbNumber);
		}

		QPushButton *pushButton = new QPushButton(i18n("LNB %1 settings", lnbNumber + 1),
							  parent);
		connect(this, SIGNAL(setDiseqcVisible(bool)), pushButton, SLOT(setVisible(bool)));
		layout->addWidget(pushButton, lnbNumber + 2, 0);

		KComboBox *comboBox = new KComboBox(parent);
		comboBox->addItem(i18n("No source"));
		comboBox->addItems(sources);
		comboBox->setCurrentIndex(sources.indexOf(config->scanSource) + 1);
		connect(this, SIGNAL(setDiseqcVisible(bool)), comboBox, SLOT(setVisible(bool)));
		layout->addWidget(comboBox, lnbNumber + 2, 1);

		diseqcConfigs.append(DvbConfig(config));
		new DvbSLnbConfigObject(spinBox, comboBox, pushButton, config);
	}

	// USALS rotor / Positions rotor

	QPushButton *pushButton = new QPushButton(i18n("LNB settings"), parent);
	connect(this, SIGNAL(setRotorVisible(bool)), pushButton, SLOT(setVisible(bool)));
	layout->addWidget(pushButton, 6, 0);

	new DvbSLnbConfigObject(spinBox, NULL, pushButton, lnbConfig);

	sourceBox = new KComboBox(parent);
	sourceBox->addItems(sources);
	connect(this, SIGNAL(setRotorVisible(bool)), sourceBox, SLOT(setVisible(bool)));
	layout->addWidget(sourceBox, 6, 1);

	satelliteView = new QTreeWidget(parent);

	// Usals rotor

	pushButton = new QPushButton(i18n("Add satellite"), parent);
	connect(this, SIGNAL(setUsalsVisible(bool)), pushButton, SLOT(setVisible(bool)));
	connect(pushButton, SIGNAL(clicked()), this, SLOT(addSatellite()));
	layout->addWidget(pushButton, 7, 0, 1, 2);

	// Positions rotor

	rotorSpinBox = new QSpinBox(parent);
	rotorSpinBox->setRange(0, 255);
	connect(this, SIGNAL(setPositionsVisible(bool)), rotorSpinBox, SLOT(setVisible(bool)));
	layout->addWidget(rotorSpinBox, 8, 0);

	pushButton = new QPushButton(i18n("Add satellite"), parent);
	connect(pushButton, SIGNAL(clicked()), this, SLOT(addSatellite()));
	connect(this, SIGNAL(setPositionsVisible(bool)), pushButton, SLOT(setVisible(bool)));
	layout->addWidget(pushButton, 8, 1);

	// USALS rotor / Positions rotor

	satelliteView->setColumnCount(2);
	satelliteView->setHeaderLabels(QStringList() << i18n("Satellite") << i18n("Position"));
	satelliteView->setIndentation(0);
	satelliteView->setMinimumHeight(100);
	satelliteView->setSortingEnabled(true);
	satelliteView->sortByColumn(0, Qt::AscendingOrder);
	connect(this, SIGNAL(setRotorVisible(bool)), satelliteView, SLOT(setVisible(bool)));
	layout->addWidget(satelliteView, 9, 0, 1, 2);

	if ((lnbConfig->configuration == DvbConfigBase::UsalsRotor) ||
	    (lnbConfig->configuration == DvbConfigBase::PositionsRotor)) {
		foreach (const DvbConfig &config, configs) {
			QStringList stringList;
			stringList << config->scanSource << QString::number(config->lnbNumber);
			satelliteView->addTopLevelItem(new QTreeWidgetItem(stringList));
		}
	}

	pushButton = new QPushButton(i18n("Remove satellite"), parent);
	connect(this, SIGNAL(setRotorVisible(bool)), pushButton, SLOT(setVisible(bool)));
	connect(pushButton, SIGNAL(clicked()), this, SLOT(removeSatellite()));
	layout->addWidget(pushButton, 10, 0, 1, 2);

	configChanged(configBox->currentIndex());
}

DvbSConfigObject::~DvbSConfigObject()
{
	delete lnbConfig;
}

void DvbSConfigObject::appendConfigs(QList<DvbConfig> &list)
{
	int index = configBox->currentIndex();

	if (index == 0) {
		// Diseqc switch

		list += diseqcConfigs;
	} else if ((index == 1) || (index == 2)) {
		// Usals rotor / Positions rotor

		for (int i = 0;; ++i) {
			QTreeWidgetItem *item = satelliteView->topLevelItem(i);

			if (item == NULL) {
				break;
			}

			QString satellite = item->text(0);

			DvbConfigBase *config = new DvbConfigBase(*lnbConfig);
			config->name = satellite;
			config->scanSource = satellite;

			if (index == 1) {
				// USALS rotor
				config->configuration = DvbConfigBase::UsalsRotor;
			} else {
				// Positions rotor
				config->configuration = DvbConfigBase::PositionsRotor;
				config->lnbNumber = item->text(1).toInt();
			}

			list.append(DvbConfig(config));
		}
	}
}

void DvbSConfigObject::configChanged(int index)
{
	if (index == 0) {
		// Diseqc switch

		emit setDiseqcVisible(true);
		emit setRotorVisible(false);
		emit setUsalsVisible(false);
		emit setPositionsVisible(false);
	} else if (index == 1) {
		// Usals rotor

		satelliteView->hideColumn(1);

		emit setDiseqcVisible(false);
		emit setRotorVisible(true);
		emit setUsalsVisible(true);
		emit setPositionsVisible(false);
	} else if (index == 2) {
		// Positions rotor

		if (satelliteView->isColumnHidden(1)) {
			int width = satelliteView->columnWidth(0);
			satelliteView->showColumn(1);
			satelliteView->setColumnWidth(0, width / 2);
		}

		emit setDiseqcVisible(false);
		emit setRotorVisible(true);
		emit setUsalsVisible(false);
		emit setPositionsVisible(true);
	}
}

void DvbSConfigObject::addSatellite()
{
	QString satellite = sourceBox->currentText();
	QString index = rotorSpinBox->text();
	QStringList stringList = QStringList() << satellite << index;

	if (configBox->currentIndex() == 1) {
		// USALS rotor

		if (satelliteView->findItems(satellite, Qt::MatchExactly).isEmpty()) {
			satelliteView->addTopLevelItem(new QTreeWidgetItem(stringList));
		}
	} else {
		// Positions rotor

		QList<QTreeWidgetItem *> items =
			satelliteView->findItems(index, Qt::MatchExactly, 1);

		if (!items.isEmpty()) {
			items.at(0)->setText(0, sourceBox->currentText());
		} else {
			satelliteView->addTopLevelItem(new QTreeWidgetItem(stringList));
		}
	}
}

void DvbSConfigObject::removeSatellite()
{
	qDeleteAll(satelliteView->selectedItems());
}

DvbConfigBase *DvbSConfigObject::createConfig(int lnbNumber)
{
	DvbConfigBase *config = new DvbConfigBase(DvbConfigBase::DvbS);
	config->timeout = 1500;
	config->configuration = DvbConfigBase::DiseqcSwitch;
	config->lnbNumber = lnbNumber;
	config->lowBandFrequency = 9750000;
	config->switchFrequency = 11700000;
	config->highBandFrequency = 10600000;
	return config;
}

DvbSLnbConfigObject::DvbSLnbConfigObject(QSpinBox *timeoutSpinBox, KComboBox *sourceBox_,
	QPushButton *configureButton_, DvbConfigBase *config_) : QObject(timeoutSpinBox),
	sourceBox(sourceBox_), configureButton(configureButton_), config(config_)
{
	connect(timeoutSpinBox, SIGNAL(valueChanged(int)), this, SLOT(timeoutChanged(int)));
	connect(configureButton, SIGNAL(clicked()), this, SLOT(configure()));

	if (sourceBox != NULL) {
		connect(sourceBox, SIGNAL(currentIndexChanged(int)),
			this, SLOT(sourceChanged(int)));
		sourceChanged(sourceBox->currentIndex());
	}

	timeoutChanged(timeoutSpinBox->value());
}

DvbSLnbConfigObject::~DvbSLnbConfigObject()
{
}

void DvbSLnbConfigObject::timeoutChanged(int value)
{
	config->timeout = value;
}

void DvbSLnbConfigObject::sourceChanged(int index)
{
	if (index <= 0) {
		// no source selected
		configureButton->setEnabled(false);
		config->name.clear();
		config->scanSource.clear();
	} else {
		configureButton->setEnabled(true);
		config->name = sourceBox->currentText();
		config->scanSource = config->name;
	}
}

void DvbSLnbConfigObject::configure()
{
	KDialog *dialog = new KDialog(configureButton);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setCaption(i18n("LNB settings"));

	QWidget *mainWidget = new QWidget(dialog);
	QGridLayout *gridLayout = new QGridLayout(mainWidget);
	dialog->setMainWidget(mainWidget);

	QButtonGroup *buttonGroup = new QButtonGroup(mainWidget);
	connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(selectType(int)));

	QRadioButton *radioButton = new QRadioButton(i18n("Universal LNB"), mainWidget);
	buttonGroup->addButton(radioButton, 1);
	gridLayout->addWidget(radioButton, 0, 0, 1, 2);

	radioButton = new QRadioButton(i18n("C-band LNB"), mainWidget);
	buttonGroup->addButton(radioButton, 2);
	gridLayout->addWidget(radioButton, 1, 0, 1, 2);

	radioButton = new QRadioButton(i18n("C-band multipoint LNB"), mainWidget);
	buttonGroup->addButton(radioButton, 3);
	gridLayout->addWidget(radioButton, 2, 0, 1, 2);

	radioButton = new QRadioButton(i18n("Custom LNB"), mainWidget);
	buttonGroup->addButton(radioButton, 4);
	gridLayout->addWidget(radioButton, 3, 0, 1, 2);

	QFrame *frame = new QFrame(mainWidget);
	frame->setFrameShape(QFrame::HLine);
	gridLayout->addWidget(frame, 4, 0, 1, 2);

	lowBandLabel = new QLabel(mainWidget);
	gridLayout->addWidget(lowBandLabel, 5, 0);

	switchLabel = new QLabel(i18n("Switch frequency (MHz)"), mainWidget);
	gridLayout->addWidget(switchLabel, 6, 0);

	highBandLabel = new QLabel(mainWidget);
	gridLayout->addWidget(highBandLabel, 7, 0);

	lowBandSpinBox = new QSpinBox(mainWidget);
	lowBandSpinBox->setRange(0, 13000);
	lowBandSpinBox->setValue(config->lowBandFrequency / 1000);
	gridLayout->addWidget(lowBandSpinBox, 5, 1);

	switchSpinBox = new QSpinBox(mainWidget);
	switchSpinBox->setRange(0, 13000);
	switchSpinBox->setValue(config->switchFrequency / 1000);
	gridLayout->addWidget(switchSpinBox, 6, 1);

	highBandSpinBox = new QSpinBox(mainWidget);
	highBandSpinBox->setRange(0, 13000);
	highBandSpinBox->setValue(config->highBandFrequency / 1000);
	gridLayout->addWidget(highBandSpinBox, 7, 1);

	gridLayout->addItem(new QSpacerItem(0, 0), 8, 0, 1, 2);
	gridLayout->setRowStretch(8, 1);

	int lnbType;

	if ((config->lowBandFrequency == 9750000) && (config->switchFrequency == 11700000) &&
	    (config->highBandFrequency == 10600000)) {
		lnbType = 1;
	} else if ((config->lowBandFrequency == 5150000) && (config->switchFrequency == 0) &&
		   (config->highBandFrequency == 0)) {
		lnbType = 2;
	} else if ((config->lowBandFrequency == 5750000) && (config->switchFrequency == 0) &&
		   (config->highBandFrequency == 5150000)) {
		lnbType = 3;
	} else {
		lnbType = 4;
	}

	currentType = 4;
	buttonGroup->button(lnbType)->setChecked(true);
	selectType(lnbType);

	connect(dialog, SIGNAL(accepted()), this, SLOT(dialogAccepted()));

	dialog->setModal(true);
	dialog->show();
}

void DvbSLnbConfigObject::selectType(int type)
{
	switch (type) {
	case 1:
		lowBandSpinBox->setValue(9750);
		switchSpinBox->setValue(11700);
		highBandSpinBox->setValue(10600);
		break;

	case 2:
		lowBandSpinBox->setValue(5150);
		switchSpinBox->setValue(0);
		highBandSpinBox->setValue(0);
		break;

	case 3:
		lowBandSpinBox->setValue(5750);
		switchSpinBox->setValue(0);
		highBandSpinBox->setValue(5150);
		break;
	}

	if (switchSpinBox->value() != 0) {
		if (currentType != 1) {
			lowBandLabel->setText(i18n("Low band LOF (MHz)"));

			switchLabel->show();
			switchSpinBox->show();

			highBandLabel->setText(i18n("High band LOF (MHz)"));
			highBandLabel->show();
			highBandSpinBox->show();
		}
	} else if (highBandSpinBox->value() != 0) {
		if (currentType != 3) {
			lowBandLabel->setText(i18n("Horizontal LOF (MHz)"));

			switchLabel->hide();
			switchSpinBox->hide();

			highBandLabel->setText(i18n("Vertical LOF (MHz)"));
			highBandLabel->show();
			highBandSpinBox->show();
		}
	} else {
		if (currentType != 2) {
			lowBandLabel->setText(i18n("LOF (MHz)"));

			switchLabel->hide();
			switchSpinBox->hide();

			highBandLabel->hide();
			highBandSpinBox->hide();
		}
	}

	if ((currentType == 4) != (type == 4)) {
		if (type == 4) {
			lowBandLabel->setEnabled(true);
			switchLabel->setEnabled(true);
			highBandLabel->setEnabled(true);
			lowBandSpinBox->setEnabled(true);
			switchSpinBox->setEnabled(true);
			highBandSpinBox->setEnabled(true);
		} else {
			lowBandLabel->setEnabled(false);
			switchLabel->setEnabled(false);
			highBandLabel->setEnabled(false);
			lowBandSpinBox->setEnabled(false);
			switchSpinBox->setEnabled(false);
			highBandSpinBox->setEnabled(false);
		}
	}

	currentType = type;
}

void DvbSLnbConfigObject::dialogAccepted()
{
	config->lowBandFrequency = lowBandSpinBox->value() * 1000;
	config->switchFrequency = switchSpinBox->value() * 1000;
	config->highBandFrequency = highBandSpinBox->value() * 1000;
}