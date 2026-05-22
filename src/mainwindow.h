#pragma once

#include <QMainWindow>
#include <QDialog>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextEdit>
#include "vtfpp/ImageFormats.h"

class QPathLineEdit;
class QLabel;
class QLineEdit;
class QComboBox;
class QSettings;

enum NormalType : bool
{
    DirectX = false,
    OpenGL
};

enum GameType : unsigned char
{
    Portal_HL2_TF2 = 0,
    GMod,
    Portal2_AlienSwarm,
    CSGO,
    Strata
};

enum PBRType : unsigned char
{
    MODEL_PBR = 0,
    BRUSH_PBR,
    ALL_PBR,
    MODEL_PHONG,
    MODEL_PHONG_ENVMAP,
    MODEL_PHONG_ENVMAP_ALPHA,
    MODEL_PHONG_ENVMAP_EMISSION,
    ALL_VLG,
    BRUSH_ENVMAP,
    BRUSH_ENVMAP_ALPHA,
    BRUSH_ENVMAP_EMISSION
};

enum AssetType : unsigned char
{
    BASE_COLOR,
    METALLIC_MAP,
    ROUGHNESS_MAP,
    AO_MAP,
    NORMAL_MAP,
    HEIGHT_MAP,
    EMISSION_MAP,
    TINT_MAP,
};

enum SuffixNames : unsigned char
{
    BaseColorName = 0,
    NormalMapName,
    EmissiveMapName,
    PhongMapName,
    EnvMapName,
    MRAOName,
    TintMapName
};

class Highlighter : public QSyntaxHighlighter
{
Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:

    QTextCharFormat typeFormat;
    QTextCharFormat braceFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat valueFormat;

};

class SuffixNameWidget : public QWidget
{
    Q_OBJECT
    static inline QMap<SuffixNames, SuffixNameWidget*> suffixNameWidgets;

    QLineEdit* suffixName;
    SuffixNames suffixNameType;
public:
    explicit SuffixNameWidget(QWidget *parent, SuffixNames nameType, const QString &fieldName);

    void setCurrentTextureName(const QString& name);
    QString getCurrentSuffixName();

    static const QMap<SuffixNames, SuffixNameWidget*>& getSuffixNameWidgets()
    {
        return suffixNameWidgets;
    }

    static QString getSuffixName(SuffixNames nameType)
    {
        return suffixNameWidgets[nameType]->getCurrentSuffixName();
    }
};

class AboutDialog : public QTextEdit
{
    explicit AboutDialog(QWidget* parent);
public:

    static void aboutOSPBR()
    {
        static auto about = new AboutDialog(nullptr);
        about->show();
    }
};

class OptionsMenu : public QDialog
{
    Q_OBJECT
    QSettings* settings;
    static inline QMap<QString, QLineEdit *> vtfMetaData;
    static inline bool isEnabled;

public:
    explicit OptionsMenu(QWidget* parent);

    void setOptionsAndDefaults();

    static QString getFullMetaData();

    static bool isKVDataEnabled();
    static void setKVEnable(bool enable)
    {
        OptionsMenu::isEnabled = enable;
    }
};

class CMainWindow : public QMainWindow
{
    Q_OBJECT
    OptionsMenu* menu;
public:
    CMainWindow();
};

class PBRAssets;

class PBRAssets : public QWidget
{
    Q_OBJECT
    static inline QMap<AssetType, PBRAssets*> assets{};
    static inline QString lastOpened;
    QLabel*     display;
    QLabel*     type;
    QPathLineEdit*  path;
    vtfpp::ImageFormat format;

    bool isRequired;
    bool valid = false;
public:
    static PBRAssets* getAsset(const AssetType& asset)
    {
        if(!assets.contains(asset))
            return nullptr;
        return assets[asset];
    }
    static const QMap<AssetType, PBRAssets*>& getAssets()
    {
        return assets;
    }
public:
    explicit PBRAssets(QWidget* parent, AssetType id, const QString &display, bool isRequired = false);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    bool setNewImageByDialog();

    [[nodiscard]] bool required() const {return this->isRequired;}

    explicit operator bool() const
    {
        return this->valid;
    };

    [[nodiscard]] vtfpp::ImageFormat getFormat() const
    {
        return this->format;
    }

    [[nodiscard]] QString getPath() const;

signals:
    void assetChanged();
};

class CPBRWindow : public QWidget
{
Q_OBJECT

    Highlighter *highlighter;

    QLineEdit *textureNameEdit;

    QComboBox *gameBox;
    QComboBox *modeBox;
    QComboBox *normalBox;
    QComboBox *targetScaleBox;
    QLineEdit *placementPrefix;
    QTextEdit *infoDisplay;
    QTextEdit *vmtViewer;
    QPushButton *saveButton;
    bool dirty = true;

    void updateVMFDisplay();

    void updateVTFDisplay();;
    void unmarkDirty() { this->dirty = false; };
    [[nodiscard]] bool isDirty() const { return this->dirty; };
public:
    void markDirty(){ this->dirty = true; }

    explicit CPBRWindow(QWidget* parent);

    void timerEvent(QTimerEvent *event) override;

    QString getPlacementPrefix();
    QString getSaveDirectory();

private slots:

    void saveTexture();

};