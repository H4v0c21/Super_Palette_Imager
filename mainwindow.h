#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QPixmap>
#include <QPainter>

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

#include <QTimer>

#include <QMessageBox>
#include <QStatusBar>
#include <qDebug>

#include <QRegExpValidator>
#include <QRegExp>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT



public:
    MainWindow(QWidget *parent = nullptr);

    QString romFilePath;
    QFile romFile;
    QByteArray romData;

    QImage paletteImage;
    QByteArray paletteData;

    QImage scaledPaletteImage;

    bool colorCountFromImage;
    quint32 imageColorCount;
    quint32 colorCount;

    quint32 paletteAddress;
    quint32 rowWidth;

    quint32 paletteImageWidth;
    quint32 paletteImageHeight;
    quint32 previewScale;

    bool quickExtract;

    ~MainWindow();

private slots:
    void on_openRomButton_clicked();
    void on_saveRomButton_clicked();
    void on_saveRomAsButton_clicked();

    void on_loadPaletteButton_clicked();

    void on_importPaletteButton_clicked();
    void on_exportPaletteButton_clicked();

    void on_importBinButton_clicked();
    void on_exportBinButton_clicked();

    void on_colorCountFromImportsCheckbox_stateChanged(int arg1);
    void on_addressBox_editingFinished();
    void on_colorCountBox_valueChanged(int arg1);
    void on_previewScaleSlider_valueChanged(int value);
    void on_addressBox_cursorPositionChanged(int arg1, int arg2);

    void resetConsoleText();

    void on_importPalButton_clicked();

    void on_exportPalButton_clicked();

    void on_actionAbout_triggered();

    void on_quickExtractCheckBox_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QTimer *consoleTextTimer;

    void getImageFromBin();
    void getPaletteBinFromROM();
    void getPalFromBin();
    void updatePalette();
    void updatePreview();
    void updateStatusMessage(QString);
};
#endif // MAINWINDOW_H
