#include "mainwindow.h"
#include "ui_mainwindow.h"

QRgb snesToRGB(quint16 snesColor)
{
    unsigned char r,g,b;

    r = (((snesColor & 0x1F) << 3) | ((snesColor >> 2) & 0x07));
    g = ((((snesColor >> 5) & 0x1F) << 3) | ((snesColor >> 7) & 0x07));
    b = ((((snesColor >> 10) & 0x1F) << 3) | ((snesColor >> 12) & 0x07));

    return qRgba(r, g, b, 255);
}

quint16 rgbToSNES(QColor color)
{
    quint16 snesColor;
    quint8 red = color.red();
    quint8 green = color.green();
    quint8 blue = color.blue();

    snesColor = (red >> 3) | ((green >> 3) << 5) | ((blue >> 3) << 10);
    return snesColor;
}

QColor snesToQcolor(quint16 snesColor)
{
    unsigned char r,g,b;

    r = (((snesColor & 0x1F) << 3) | ((snesColor >> 2) & 0x07));
    g = ((((snesColor >> 5) & 0x1F) << 3) | ((snesColor >> 7) & 0x07));
    b = ((((snesColor >> 10) & 0x1F) << 3) | ((snesColor >> 12) & 0x07));

    return QColor(r, g, b);
}


quint32 hexStringToInt(QString string)
{
    bool convertOK;
    quint32 integer = string.toInt(&convertOK, 16);

    if (convertOK) {
        return integer;
    }
    else
    {
        return 0;
    }
}

quint32 stringToInt(QString string)
{
    bool convertOK;
    quint32 integer = string.toInt(&convertOK, 10);

    if (convertOK) {
        return integer;
    }
    else
    {
        return 0;
    }
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QRegExp hexRegex("[0-9A-Fa-f]{6}");
    QRegExpValidator validator(hexRegex, ui->addressBox);
    ui->addressBox->setValidator(&validator);

    ui->colorCountBox->setValue(128);
    colorCountFromImage = false;

    ui->addressMapModeBox->addItem("None");
    ui->addressMapModeBox->addItem("Lo ROM");
    ui->addressMapModeBox->addItem("Hi ROM");
    ui->addressMapModeBox->addItem("ExLo ROM");
    ui->addressMapModeBox->addItem("ExHi ROM");
    ui->addressMapModeBox->addItem("SA-1 ROM");
    ui->addressMapModeBox->addItem("SDD-1 ROM");

    consoleTextTimer = new QTimer(this);
    connect(consoleTextTimer, &QTimer::timeout, this, &MainWindow::resetConsoleText);
    consoleTextTimer->setInterval(5000);

    paletteImageWidth = 16;
    paletteImageHeight = 16;
    previewScale = 16;
    paletteImage = QImage(paletteImageWidth, paletteImageHeight, QImage::Format_RGB32);
    paletteImage.fill(Qt::white);

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::updateStatusMessage(QString messageString)
{
    qDebug() << messageString;
    statusBar()->showMessage(messageString, 10000);
    //consoleTextTimer->start();
}



void MainWindow::updatePalette()
{
    if (!romData.isEmpty())
    {
        if (!romFilePath.isEmpty())
        {
            paletteAddress = hexStringToInt(ui->addressBox->text());
            colorCount = ui->colorCountBox->value();
            getPaletteBinFromROM();
            getImageFromBin();
        }
    }
}



void MainWindow::updatePreview()
{
    if (!paletteImage.isNull())
    {
        scaledPaletteImage = paletteImage.scaledToWidth(paletteImage.width() * previewScale);
        ui->paletteImageDisplay->setPixmap(QPixmap::fromImage(scaledPaletteImage));
    }
}



void MainWindow::getPaletteBinFromROM()
{
    if (!romData.isEmpty())
    {
        if (romData.size() > paletteAddress + (colorCount * 2))
        {
            paletteData = romData.mid(paletteAddress, colorCount*2);
            return;
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        updateStatusMessage("ERROR: No ROM file opened.");
        return;
    }
}



void MainWindow::getImageFromBin()
{
    if (!paletteData.isEmpty())
    {
        rowWidth = 16;

        QDataStream paletteBufferStream(paletteData);
        paletteBufferStream.setByteOrder(QDataStream::LittleEndian);

        if (colorCount < rowWidth)
        {
            paletteImageWidth = colorCount;
            paletteImageHeight = 1;
        }
        else
        {
            if (colorCount % rowWidth != 0)
            {
                paletteImageWidth = rowWidth;
                paletteImageHeight = colorCount / rowWidth + 1;
            }
            else
            {
                paletteImageWidth = rowWidth;
                paletteImageHeight = colorCount / rowWidth;
            }
        }

        paletteImage = QImage(paletteImageWidth, paletteImageHeight, QImage::Format_RGB32);
        paletteImage.fill(Qt::white);

        quint16 snesColor;
        QRgb color;

        for (int i = 0; i < colorCount; i++)
        {
            paletteBufferStream >> snesColor;
            color = snesToRGB(snesColor);
            paletteImage.setPixel(i % rowWidth, i / rowWidth, color);
        }
    }
    else
    {
        updateStatusMessage("ERROR: No palette data to process.");
        return;
    }
}



void MainWindow::resetConsoleText()
{
    consoleTextTimer->stop();
}



void MainWindow::on_openRomButton_clicked()
{
    QString oldRomFilePath = romFilePath;
    romFilePath = QFileDialog::getOpenFileName(this, tr("Open ROM file"), QDir::homePath(), tr("SNES ROMs (*.sfc *.smc)"));
    QFile romFile(romFilePath);

    if (!romFilePath.isEmpty())
    {
        if (romFile.open(QIODevice::ReadOnly))
        {
            romData = romFile.readAll();
            romFile.close();

            ui->saveRomButton->setEnabled(true);
            ui->saveRomAsButton->setEnabled(true);
            ui->exportBinButton->setEnabled(true);
            ui->exportPalButton->setEnabled(true);
            ui->exportPaletteButton->setEnabled(true);
            ui->importBinButton->setEnabled(true);
            ui->importPalButton->setEnabled(true);
            ui->importPaletteButton->setEnabled(true);
            ui->loadPaletteButton->setEnabled(true);

            ui->romPathLabel->setText(romFilePath);
            updateStatusMessage("SUCCESS: Opened ROM file.");
        }
        else
        {
            ui->saveRomButton->setEnabled(false);
            ui->saveRomAsButton->setEnabled(false);
            ui->exportBinButton->setEnabled(false);
            ui->exportPalButton->setEnabled(false);
            ui->exportPaletteButton->setEnabled(false);
            ui->importBinButton->setEnabled(false);
            ui->importPalButton->setEnabled(false);
            ui->importPaletteButton->setEnabled(false);
            ui->loadPaletteButton->setEnabled(false);

            romFilePath = oldRomFilePath;
            ui->romPathLabel->setText(romFilePath);
            updateStatusMessage("ERROR: Failed to open ROM file.");
            return;
        }
    }
    else
    {
        romFilePath = oldRomFilePath;
        ui->romPathLabel->setText(romFilePath);
        updateStatusMessage("ERROR: No ROM path provided.");
        return;
    }
}



void MainWindow::on_saveRomButton_clicked()
{
    if (!romData.isEmpty())
    {
        if (!romFilePath.isEmpty())
        {
            QFile outputRomFile(romFilePath);

            if (outputRomFile.open(QIODevice::WriteOnly))
            {
                QDataStream outputRomStream(&outputRomFile);
                outputRomStream.writeRawData(romData.constData(), romData.size());
                outputRomFile.close();
                updateStatusMessage("SUCCESS: Saved ROM.");
            }
            else
            {
                updateStatusMessage("ERROR: Failed to save ROM.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: No ROM path provided.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_saveRomAsButton_clicked()
{
    if (!romData.isEmpty())
    {
        QString filePath = QFileDialog::getSaveFileName(this, "Save ROM file", "", "SNES ROMs (*.sfc *.smc)");

        if (!filePath.isEmpty())
        {
            QFile outputRomFile(filePath);

            if (outputRomFile.open(QIODevice::WriteOnly))
            {
                QDataStream outputRomStream(&outputRomFile);
                outputRomStream.writeRawData(romData.constData(), romData.size());
                outputRomFile.close();
                updateStatusMessage("SUCCESS: Saved ROM.");
            }
            else
            {
                updateStatusMessage("ERROR: Failed to save ROM.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: No ROM path provided.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_loadPaletteButton_clicked()
{
    if (!romData.isEmpty())
    {
        if (ui->addressBox->hasAcceptableInput())
        {
            updatePalette();
            updatePreview();
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
    }
}



void MainWindow::on_importPaletteButton_clicked()
{
    if (!romData.isEmpty())
    {
        if (ui->addressBox->hasAcceptableInput())
        {
            QString paletteImagePath = QFileDialog::getOpenFileName(this, tr("Open Palette Image"), QDir::homePath(), tr("Images (*.png *.bmp)"));
            QImage newPaletteImage = QImage(paletteImagePath);

            if (!newPaletteImage.isNull())
            {
                paletteAddress = hexStringToInt(ui->addressBox->text());

                colorCount = ui->colorCountBox->value();

                paletteImageWidth = newPaletteImage.width();
                paletteImageHeight = newPaletteImage.height();

                imageColorCount = paletteImageWidth * paletteImageHeight;

                if (colorCountFromImage == true || imageColorCount < colorCount)
                {
                    ui->colorCountBox->setValue(imageColorCount);
                    colorCount = imageColorCount;
                }

                if (romData.size() > paletteAddress + (colorCount *2))
                {
                    QDataStream romBufferStream(&romData, QIODevice::WriteOnly);
                    romBufferStream.setByteOrder(QDataStream::LittleEndian);
                    romBufferStream.device()->seek(paletteAddress);

                    int x = 0;
                    int y = 0;
                    QColor pixelColor;
                    quint16 snesColor;

                    for (int i = 0; i < colorCount; i++)
                    {
                        x = i % rowWidth;
                        y = i / rowWidth;

                        pixelColor = newPaletteImage.pixelColor(x, y);
                        snesColor = rgbToSNES(pixelColor);
                        romBufferStream << snesColor;
                    }

                    updatePalette();
                    updatePreview();

                    updateStatusMessage("SUCCESS: Imported palette to ROM.");
                }
                else
                {
                    updateStatusMessage("ERROR: Invalid palette address.");
                    return;
                }
            }
            else
            {
                updateStatusMessage("ERROR: Failed to open image file.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_exportPaletteButton_clicked()
{
    QString filePath;

    if (!romData.isEmpty())
    {
        if (ui->addressBox->hasAcceptableInput())
        {
            if (quickExtract == true)
            {
                QFileInfo romInfo(romFilePath);
                QString romName = romInfo.fileName();
                QString romFolderPath = romInfo.absolutePath();
                filePath = romFolderPath + "/" + romName + "-$" + QString::number(paletteAddress, 16) + ".png";
            }
            else
            {
                filePath = QFileDialog::getSaveFileName(this, "Save as Palette Image", "", "Images (*.png *.bmp)");
            }

            if (!paletteImage.isNull())
            {
                if (!filePath.isEmpty())
                {

                    if (paletteImage.save(filePath))
                    {
                        updateStatusMessage("SUCCESS: Exported palette to image.");
                    }
                    else
                    {
                        updateStatusMessage("ERROR: Failed to save palette to image.");
                        return;
                    }
                }
                else
                {
                    updateStatusMessage("ERROR: No image path provided.");
                    return;
                }
            }
            else
            {
                updateStatusMessage("ERROR: No palette data to export.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_importBinButton_clicked()
{
    if (!romData.isEmpty())
    {
        if (ui->addressBox->hasAcceptableInput())
        {
            QString binFilePath = QFileDialog::getOpenFileName(this, tr("Open Raw Palette Data"), QDir::homePath(), tr("SNES Palettes (*.bin)"));
            QFile binFile(binFilePath);

            if (!binFilePath.isEmpty())
            {
                if (binFile.open(QIODevice::ReadOnly))
                {
                    QByteArray binData = binFile.readAll();
                    quint32 binColorCount = binData.size() / 2;
                    binFile.close();

                    if (colorCountFromImage == true || binColorCount < colorCount)
                    {
                        ui->colorCountBox->setValue(binColorCount);
                        colorCount = binColorCount;
                    }


                    if (romData.size() > paletteAddress + (colorCount * 2))
                    {
                        romData.replace(paletteAddress, colorCount * 2, binData.mid(0, colorCount * 2));
                        updatePalette();
                        updatePreview();

                        updateStatusMessage("SUCCESS: Imported palette to ROM.");
                    }
                    else
                    {
                        updateStatusMessage("ERROR: Invalid palette address.");
                        return;
                    }
                }
                else
                {
                    updateStatusMessage("ERROR: Failed to open palette data file.");
                    return;
                }
            }
            else
            {
                updateStatusMessage("ERROR: No palette data path provided.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_exportBinButton_clicked()
{
    QString filePath;
    if (!romData.isEmpty())
    {
        if (ui->addressBox->hasAcceptableInput())
        {
            if (quickExtract == true)
            {
                QFileInfo romInfo(romFilePath);
                QString romName = romInfo.fileName();
                QString romFolderPath = romInfo.absolutePath();
                filePath = romFolderPath + "/" + romName + "-$" + QString::number(paletteAddress, 16) + ".bin";
            }
            else
            {
                filePath = QFileDialog::getSaveFileName(this, "Save as Raw Palette Data", "", "Binary Data (*.bin)");
            }

            if (!filePath.isEmpty())
            {
                QFile outputBinFile(filePath);

                if (outputBinFile.open(QIODevice::WriteOnly))
                {
                    if (romData.size() > paletteAddress + (colorCount * 2))
                    {
                        QDataStream outputBinStream(&outputBinFile);
                        QByteArray binData = romData.mid(paletteAddress, colorCount * 2);
                        outputBinStream.writeRawData(binData.constData(), binData.size());
                        outputBinFile.close();
                        updateStatusMessage("SUCCESS: Saved raw palette data.");
                    }
                    else
                    {
                        updateStatusMessage("ERROR: Invalid palette address.");
                        return;
                    }
                }
                else
                {
                    updateStatusMessage("ERROR: Failed to save palette data.");
                    return;
                }
            }
            else
            {
                updateStatusMessage("ERROR: No ROM path provided.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_importPalButton_clicked()
{
    if (!romData.isEmpty())
    {
        if (ui->addressBox->hasAcceptableInput())
        {
            QString palFilePath = QFileDialog::getOpenFileName(this, tr("Open .PAL file"), QDir::homePath(), tr("SNES Palettes (*.pal)"));

            if (!palFilePath.isEmpty())
            {
                QFile palFile(palFilePath);

                paletteAddress = hexStringToInt(ui->addressBox->text());
                colorCount = ui->colorCountBox->value();

                if (palFile.open(QIODevice::ReadOnly))
                {
                    QByteArray palData = palFile.readAll();
                    quint32 palColorCount = palData.size() / 3;
                    palFile.close();

                    if (colorCountFromImage == true || palColorCount < colorCount)
                    {
                        ui->colorCountBox->setValue(palColorCount);
                        colorCount = palColorCount;
                    }

                    if (romData.size() > paletteAddress + (colorCount * 2))
                    {
                        QDataStream romBufferStream(&romData, QIODevice::WriteOnly);
                        romBufferStream.setByteOrder(QDataStream::LittleEndian);
                        romBufferStream.device()->seek(paletteAddress);

                        QDataStream palDataStream(&palData, QIODevice::ReadOnly);

                        QColor color;
                        quint8 red;
                        quint8 green;
                        quint8 blue;
                        quint16 snesColor;

                        for (int i = 0; i < colorCount; i++)
                        {
                            palDataStream >> red;
                            palDataStream >> green;
                            palDataStream >> blue;

                            color.setRed(red);
                            color.setGreen(green);
                            color.setBlue(blue);

                            snesColor = rgbToSNES(color);

                            romBufferStream << snesColor;
                        }

                        updatePalette();
                        updatePreview();

                        updateStatusMessage("SUCCESS: Imported palette to ROM.");
                    }
                    else
                    {
                        updateStatusMessage("ERROR: Invalid palette address.");
                        return;
                    }
                }
                else
                {
                    updateStatusMessage("ERROR: Failed to open .pal file.");
                    return;
                }
            }
            else
            {
                updateStatusMessage("ERROR: No .pal path provided.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_exportPalButton_clicked()
{
    QString filePath;
    if (!romData.isEmpty())
    {

        if (ui->addressBox->hasAcceptableInput())
        {
            if (!paletteData.isEmpty())
            {
                if (quickExtract == true)
                {
                    QFileInfo romInfo(romFilePath);
                    QString romName = romInfo.fileName();
                    QString romFolderPath = romInfo.absolutePath();
                    filePath = romFolderPath + "/" + romName + "-$" + QString::number(paletteAddress, 16) + ".pal";
                }
                else
                {
                    filePath = QFileDialog::getSaveFileName(this, "Save as .PAL file", "", "SNES Palettes (*.pal)");
                }

                if (!filePath.isEmpty())
                {
                    QFile outputPalFile(filePath);

                    if (outputPalFile.open(QIODevice::WriteOnly))
                    {
                        if (romData.size() > paletteAddress + (colorCount * 2))
                        {
                            QDataStream outputPalStream(&outputPalFile);
                            QByteArray paletteData = romData.mid(paletteAddress, colorCount * 2);

                            QDataStream paletteBufferStream(paletteData);
                            paletteBufferStream.setByteOrder(QDataStream::LittleEndian);

                            quint16 snesColor;
                            QColor color;

                            for (int i = 0; i < colorCount; i++)
                            {
                                paletteBufferStream >> snesColor;
                                color = snesToQcolor(snesColor);
                                outputPalStream << static_cast<quint8>(color.red());
                                outputPalStream << static_cast<quint8>(color.green());
                                outputPalStream << static_cast<quint8>(color.blue());
                            }

                            outputPalFile.close();
                            updateStatusMessage("SUCCESS: Saved .pal data.");
                        }
                        else
                        {
                            updateStatusMessage("ERROR: Invalid palette address.");
                            return;
                        }
                    }
                    else
                    {
                        updateStatusMessage("ERROR: Failed to save pal data.");
                        return;
                    }
                }
                else
                {
                    updateStatusMessage("ERROR: No ROM path provided.");
                    return;
                }
            }
            else
            {
                updateStatusMessage("ERROR: No palette data to process.");
                return;
            }
        }
        else
        {
            updateStatusMessage("ERROR: Invalid palette address.");
            return;
        }
    }
    else
    {
        QMessageBox::warning(nullptr, "Warning", "A ROM file must be loaded before performing this action!");
        updateStatusMessage("ERROR: No ROM loaded.");
        return;
    }
}



void MainWindow::on_colorCountFromImportsCheckbox_stateChanged(int arg1)
{
    if (ui->colorCountFromImportsCheckbox->checkState())
    {
        colorCountFromImage = true;
    }
    else
    {
        colorCountFromImage = false;
    }
}



void MainWindow::on_addressBox_editingFinished()
{
    updatePalette();
    updatePreview();
}



void MainWindow::on_colorCountBox_valueChanged(int arg1)
{
    updatePalette();

    if (!ui->addressBox->text().isEmpty())
    {
        updatePreview();
    }
}



void MainWindow::on_previewScaleSlider_valueChanged(int value)
{
    previewScale = value;
    ui->paletteImageDisplay->adjustSize();
    //ui->paletteImageDisplay->resize(previewScale, previewScale);
    //updatePalette();
    updatePreview();
}



void MainWindow::on_addressBox_cursorPositionChanged(int arg1, int arg2)
{
    if (ui->addressBox->text().isEmpty())
    {
        //ui->addressBox->setText("");
        ui->addressBox->setCursorPosition(0);
    }
}



void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(nullptr, "About",
"Super Palette Imager Version: 1.1.0.\n\n"

"Copyright © 2024 Alex Corley (H4v0c21).\n\n"

"Super Palette Imager is a program designed to edit the color palettes of SNES games primarily through the use of importing/exporting color palettes as images."
);
}



void MainWindow::on_quickExtractCheckBox_stateChanged(int arg1)
{
    if (ui->quickExtractCheckBox->checkState())
    {
        quickExtract = true;
    }
    else
    {
        quickExtract = false;
    }
}
