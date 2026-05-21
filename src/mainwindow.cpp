#include "mainwindow.h"

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QFileDialog>
#include <QMouseEvent>
#include <QMimeData>
#include <vtfpp/vtfpp.h>
#include <QGroupBox>
#include <QComboBox>
#include <QTextEdit>
#include <QStyleHints>
#include <QMessageBox>
#include <QPushButton>
#include <QMenuBar>
#include <QSettings>
#include <cmath>

constexpr char supported_images[] = "*.png *.jpg *.jpeg *.exr *.bmp *.psd *.gif *.pic *.hdr *.tga *.qoi *.webp";

constexpr char baseColor[]	= "basecolor";
constexpr char normalMap[] = "normal";
constexpr char emissionMask[] = "emit";
constexpr char phongMask[] = "phongexp";
constexpr char envMask[] = "envmask";
constexpr char metalRoughnessAO[] = "mrao";
constexpr char tintMap[] = "tint";

QMap<vtfpp::ImageFormat, QString> IMAGE_FORMATS = {
        { vtfpp::ImageFormat::RGBA8888, "RGBA8888" },
        { vtfpp::ImageFormat::ABGR8888, "ABGR8888" },
        { vtfpp::ImageFormat::RGB888, "RGB888" },
        { vtfpp::ImageFormat::BGR888, "BGR888" },
        { vtfpp::ImageFormat::RGB565, "RGB565" },
        { vtfpp::ImageFormat::I8, "I8" },
        { vtfpp::ImageFormat::IA88, "IA88" },
        { vtfpp::ImageFormat::P8, "P8" },
        { vtfpp::ImageFormat::A8, "A8" },
        { vtfpp::ImageFormat::RGB888_BLUESCREEN, "RGB888_BLUESCREEN" },
        { vtfpp::ImageFormat::BGR888_BLUESCREEN, "BGR888_BLUESCREEN" },
        { vtfpp::ImageFormat::ARGB8888, "ARGB8888" },
        { vtfpp::ImageFormat::BGRA8888, "BGRA8888" },
        { vtfpp::ImageFormat::DXT1, "DXT1" },
        { vtfpp::ImageFormat::DXT3, "DXT3" },
        { vtfpp::ImageFormat::DXT5, "DXT5" },
        { vtfpp::ImageFormat::BGRX8888, "BGRX8888" },
        { vtfpp::ImageFormat::BGR565, "BGR565" },
        { vtfpp::ImageFormat::BGRX5551, "BGRX5551" },
        { vtfpp::ImageFormat::BGRA4444, "BGRA4444" },
        { vtfpp::ImageFormat::DXT1_ONE_BIT_ALPHA, "DXT1_ONEBITALPHA" },
        { vtfpp::ImageFormat::BGRA5551, "BGRA5551" },
        { vtfpp::ImageFormat::UV88, "UV88" },
        { vtfpp::ImageFormat::UVWQ8888, "UVWQ8888" },
        { vtfpp::ImageFormat::RGBA16161616F, "RGBA16161616F" },
        { vtfpp::ImageFormat::RGBA16161616, "RGBA16161616" },
        { vtfpp::ImageFormat::UVLX8888, "UVLX8888" },
        { vtfpp::ImageFormat::R32F, "R32F" },
        { vtfpp::ImageFormat::RGB323232F, "RGB323232F" },
        { vtfpp::ImageFormat::RGBA32323232F, "RGBA32323232F" },
        //	{ vtfpp::ImageFormat::NV_DST16, "NV_DST16" },
        //	{ vtfpp::ImageFormat::NV_DST24, "NV_DST24" },
        //	{ vtfpp::ImageFormat::NV_INTZ, "NV_INTZ" },
        //	{ vtfpp::ImageFormat::NV_RAWZ, "NV_RAWZ" },
        //	{ vtfpp::ImageFormat::ATI_DST16, "ATI_DST16" },
        //	{ vtfpp::ImageFormat::ATI_DST24, "ATI_DST24" },
        { vtfpp::ImageFormat::EMPTY, "NV_NULL" },
        { vtfpp::ImageFormat::ATI2N, "ATI2N" },
        { vtfpp::ImageFormat::ATI1N, "ATI1N" },
        //	{ vtfpp::ImageFormat::ATI2N_OLD, "ATI2N Old" },
        //	{ vtfpp::ImageFormat::ATI1N_OLD, "ATI1N Old" },
        { vtfpp::ImageFormat::STRATA_BC6H, "BC6H" },
        { vtfpp::ImageFormat::STRATA_BC7, "BC7" } };

const QPixmap getUploadImage()
{
    static const QPixmap uploadImageDark = {":/upload_image_dark.png"};
    static const QPixmap uploadImageLight = {":/upload_image_light.png"};
    static const QPixmap uploadImageUnknown = {":/upload_image_unknown.png"};

    if(qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark)
        return uploadImageDark;
    if(qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Light)
        return uploadImageLight;

    return uploadImageUnknown;
}

class QPathLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    using QLineEdit::QLineEdit;

    void mouseDoubleClickEvent(QMouseEvent *) override
    {
        emit onDoubleClicked();
    };
    void mousePressEvent(QMouseEvent *event) override
    {
        if(event->button() == Qt::RightButton)
        {
            emit onRightClicked();
        }
    };

    signals:
    void onDoubleClicked();
    void onRightClicked();
};

CMainWindow::CMainWindow() : QMainWindow()
{
    auto centralWidget = new CPBRWindow(this);
    this->setCentralWidget(centralWidget);

    this->menu = new OptionsMenu(this);

    this->menuBar()->addAction(tr("Options"), this->menu, &OptionsMenu::exec);
    connect(this->menu, &OptionsMenu::finished, centralWidget, &CPBRWindow::markDirty);

}

void applyDisplaySettings(QLabel* label)
{
    label->setFixedSize(64,64);
    label->setPixmap(getUploadImage());
}

CPBRWindow::CPBRWindow(QWidget *parent) : QWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    auto substanceLayout = new QHBoxLayout();
    substanceLayout->setAlignment(Qt::AlignLeft);
    auto fields = new QGroupBox(tr("Input"),this);

    auto fieldsLayout = new QVBoxLayout(fields);
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::BASE_COLOR, tr("Base Color"), true));
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::METALLIC_MAP, tr("Metallic")));
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::ROUGHNESS_MAP, tr("Roughness"), true));
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::AO_MAP, tr("Ambient Occlusion")));
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::NORMAL_MAP, tr("Normal Map")));
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::HEIGHT_MAP, tr("Height Map")));
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::EMISSION_MAP, tr("Emission")));
    fieldsLayout->addWidget(new PBRAssets(this, AssetType::TINT_MAP, tr("Tint Map")));
    substanceLayout->addWidget(fields);

    for(auto asset : PBRAssets::getAssets())
        connect(asset, &PBRAssets::assetChanged, this, &CPBRWindow::markDirty);

    auto options = new QGroupBox(tr("Output"),this);
    auto optionsLayout = new QGridLayout(options);
    optionsLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto nameLabelLayout = new QHBoxLayout();
    auto vmtNameLabel = new QLabel(tr("VMT Name"),this);
    nameLabelLayout->addWidget(vmtNameLabel, Qt::AlignLeft);
    auto requiredField = new QLabel(tr("(Required)"),this);
    requiredField->setDisabled(true);
    nameLabelLayout->addWidget(requiredField, Qt::AlignLeft);
    nameLabelLayout->setContentsMargins(1,1,1,1);
    optionsLayout->addLayout(nameLabelLayout,0,0);

    this->textureNameEdit = new QLineEdit(options);
    optionsLayout->addWidget(this->textureNameEdit,1,0);
    connect(this->textureNameEdit, &QLineEdit::textChanged, this, &CPBRWindow::markDirty);

    optionsLayout->addWidget(new QLabel(tr("Game"),this),2,0);

    this->gameBox = new QComboBox(options);
    this->gameBox->addItem(tr("HL2:E2/Portal/TF2"),GameType::Portal_HL2_TF2);
    this->gameBox->addItem(tr("Portal 2/Alien Swarm"), GameType::Portal2_AlienSwarm);
    this->gameBox->addItem(tr("Garry's Mod"),GameType::GMod);
    this->gameBox->addItem(tr("CS:GO"),GameType::CSGO);
    this->gameBox->addItem(tr("Strata Source"),GameType::Strata);
    optionsLayout->addWidget(this->gameBox,3,0);
    connect(this->gameBox, &QComboBox::currentIndexChanged, this, &CPBRWindow::markDirty);

    optionsLayout->addWidget(new QLabel(tr("Mode"),this),4,0);

    this->modeBox = new QComboBox(options);
    this->modeBox->addItem(tr("Model: PBR"), PBRType::MODEL_PBR);
    this->modeBox->addItem(tr("Model: Phong"), PBRType::MODEL_PHONG);
    this->modeBox->addItem(tr("Model: Phong+Envmap"), PBRType::MODEL_PHONG_ENVMAP);
    this->modeBox->addItem(tr("Model: Phong+Envmap+Alpha"), PBRType::MODEL_PHONG_ENVMAP_ALPHA);
    this->modeBox->addItem(tr("Model: Phong+Envmap+Emission"), PBRType::MODEL_PHONG_ENVMAP_EMISSION);
    this->modeBox->addItem(tr("Brush: PBR"), PBRType::BRUSH_PBR);
    this->modeBox->addItem(tr("Brush: Envmap"), PBRType::BRUSH_ENVMAP);
    this->modeBox->addItem(tr("Brush: Envmap+Alpha"), PBRType::BRUSH_ENVMAP_ALPHA);
    this->modeBox->addItem(tr("Brush: Envmap+Emission"), PBRType::BRUSH_ENVMAP_EMISSION);
    optionsLayout->addWidget(this->modeBox,5,0);
    connect(this->modeBox, &QComboBox::currentIndexChanged, this, &CPBRWindow::markDirty);

    optionsLayout->addWidget(new QLabel(tr("Normal Map Type"),this),6,0);

    this->normalBox = new QComboBox(options);
    this->normalBox->addItem(tr("DirectX/3DX Max"),NormalType::DirectX);
    this->normalBox->addItem(tr("OpenGL/Maya/Blender"),NormalType::OpenGL);
    optionsLayout->addWidget(this->normalBox,7,0);
    connect(this->normalBox, &QComboBox::currentIndexChanged, this, &CPBRWindow::markDirty);

    optionsLayout->addWidget(new QLabel(tr("Target Scale"),this),8,0);

    this->targetScaleBox = new QComboBox(options);
    optionsLayout->addWidget(this->targetScaleBox,9,0);
    this->targetScaleBox->addItem(tr("None"), -1);
    connect(this->targetScaleBox, &QComboBox::currentIndexChanged, this, &CPBRWindow::markDirty);
    for(int i = 4096; i > 32; i /= 2 )
    {
        this->targetScaleBox->addItem(QString::number(i) + "x", i);
    }
    optionsLayout->addWidget(new QLabel(tr("Texture Path (for example: models/props_bts)"),this),10,0);

    this->placementPrefix = new QLineEdit(options);
    optionsLayout->addWidget(this->placementPrefix,11,0);
    connect(this->placementPrefix, &QLineEdit::textChanged, this, &CPBRWindow::markDirty);

    optionsLayout->addWidget(new QLabel(tr("VTF Output"), this),12,0, Qt::AlignCenter);

    this->infoDisplay = new QTextEdit(options);
    this->infoDisplay->setReadOnly(true);
    optionsLayout->addWidget(this->infoDisplay, 13, 0);

    substanceLayout->addWidget(options, Qt::AlignLeft);
    auto vmtGroup = new QGroupBox(tr("VMT Viewer"),this);
    auto vmtViewerLayout = new QVBoxLayout(vmtGroup);
    this->vmtViewer = new QTextEdit(this);
    this->vmtViewer->setReadOnly(true);
    this->highlighter = new Highlighter(this->vmtViewer->document());
    vmtViewerLayout->addWidget(this->vmtViewer);
    substanceLayout->addWidget(vmtGroup, Qt::AlignRight);
    this->startTimer(100);
    layout->addLayout(substanceLayout);

    auto buttons = new QDialogButtonBox(this);
    saveButton = buttons->addButton("Save", QDialogButtonBox::AcceptRole);
    buttons->addButton("Close", QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);
    connect(buttons, SIGNAL(rejected()), this->parent(), SLOT(close()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(saveTexture()));
}

void shaderType(PBRType type, QString& vmt)
{
    switch (type) {
        case MODEL_PBR:
            vmt = vmt.arg("PBR","$model\t1\n\t%1");
            break;
        case MODEL_PHONG:
        case MODEL_PHONG_ENVMAP:
        case MODEL_PHONG_ENVMAP_ALPHA:
        case MODEL_PHONG_ENVMAP_EMISSION:
            vmt=vmt.arg("VertexLitGeneric");
            break;
        case BRUSH_PBR:
            vmt=vmt.arg("PBR");
            break;
        case BRUSH_ENVMAP:
        case BRUSH_ENVMAP_ALPHA:
        case BRUSH_ENVMAP_EMISSION:
            vmt=vmt.arg("LightmappedGeneric");
            break;
    }
}

QString CPBRWindow::getPlacementPrefix()
{
    if(this->placementPrefix->text().isEmpty())
        return "";
    return this->placementPrefix->text() + "/";
}

float envmapTint(GameType type, bool vlg)
{
    if(type > Portal2_AlienSwarm)
        return 1.0f;
    return vlg ? 0.1f : std::round(std::pow(0.1f,2.2f) * 1000) / 1000;
}

float lightScale(GameType type)
{
    if(type == CSGO || type == Strata || type == GMod)
        return 1.0f;
    return -1.0f;
}

void genericParameters(PBRType type, GameType game, const QString &pathName, QString& vmt) {

    vmt=vmt.arg("$basetexture\t\"%1\"\n\t%2").arg(pathName + "_" + SuffixNameWidget::getSuffixName(BaseColorName));

    if(PBRAssets::getAsset(AssetType::NORMAL_MAP)->operator bool())
        vmt=vmt.arg("$bumpmap\t\t\"%1\"\n\t%2").arg(pathName + "_" + SuffixNameWidget::getSuffixName(NormalMapName));

    if(type < PBRType::ALL_PBR)
    {
        vmt=vmt.arg("$mraotexture\t\"%1\"\n\tmraoscale\t\"[1 1 1]\"\n\t%2").arg(pathName + "_" + SuffixNameWidget::getSuffixName(MRAOName));
    }

    if((type < ALL_PBR && vtfpp::ImageFormatDetails::transparent(PBRAssets::getAsset(BASE_COLOR)->getFormat())))
    {
        vmt=vmt.arg("$alphatest\t1\n\t%1");
    }

    bool hasTint = PBRAssets::getAsset(TINT_MAP)->operator bool();

    if(!hasTint && (type == PBRType::MODEL_PHONG_ENVMAP_ALPHA || type == PBRType::BRUSH_ENVMAP_ALPHA) )
    {
        vmt=vmt.arg("$transparency\t1\n\t%1");
    }

    if(PBRAssets::getAsset(AssetType::HEIGHT_MAP)->operator bool() && game == Strata)
    {
        vmt=vmt.arg("$parallax\t1\n\t%1");
        vmt=vmt.arg("$parallaxdepth\t0.4\n\t%1");
        vmt=vmt.arg("$parallaxcenter\t0.5\n\t%1");
    }

    if(type < ALL_PBR)
        return;

    auto envmaptint = envmapTint(game, type >= MODEL_PHONG && type <= MODEL_PHONG_ENVMAP_EMISSION);

    if( type >= PBRType::MODEL_PHONG_ENVMAP )
    {
        vmt=vmt.arg("$envmap\t\"env_cubemap\"\n\t%1");
        vmt=vmt.arg("$envmaptint\t\"[%1 %1 %1]\"\n\t%2").arg(envmaptint);
        vmt=vmt.arg("$envmapcontrast\t1.0\n\t%1");
    }

    if(type == BRUSH_ENVMAP || type == PBRType::MODEL_PHONG_ENVMAP)
    {
        if(game == GMod && type == MODEL_PHONG_ENVMAP)
        {
            vmt=vmt.arg("$normalmapalphaenvmapmask\t1\n\t%1");
            vmt=vmt.arg("$basemapalphaphongmask\t1\n\t%1");
        }
        else
        {
            vmt=vmt.arg("$basetextureenvmapmask\t1\n\t%1");
        }
    }
    else
    {
        vmt=vmt.arg("$envmapmask\t\"%1\"\n\t%2").arg(pathName + "_" + SuffixNameWidget::getSuffixName(EnvMapName));
        if(type > PBRType::ALL_PBR && type < PBRType::ALL_VLG)
        {
            vmt=vmt.arg("$envmapfresnel\t1\n\t%1");
        }
        else
        {
            vmt=vmt.arg("$fresnelreflection\t0\n\t%1");
        }
    }

    auto lightscale = lightScale(game);
    if(lightscale > 0.0f)
    {
        vmt=vmt.arg("$envmaplightscale\t%1\n\t%2").arg(lightscale);
    }

    if(type > PBRType::ALL_PBR && type < PBRType::ALL_VLG )
    {
        vmt=vmt.arg("$phong\t1\n\t%1");
        vmt=vmt.arg("$phongexponenttexture\t\"%1\"\n\t%2").arg(pathName + "_" +
                                                                       SuffixNameWidget::getSuffixName(PhongMapName));
        vmt=vmt.arg("$phongexponentfactor\t32.0\n\t%1");
        vmt=vmt.arg("$phongboost\t5.0\n\t%1");
    }

    if((type >= PBRType::MODEL_PHONG && type <= PBRType::MODEL_PHONG_ENVMAP_EMISSION) || (type >= BRUSH_ENVMAP && type <= BRUSH_ENVMAP_EMISSION))
    {
        vmt=vmt.arg("$phongfresnelranges\t\"[0.1 0.8 1.0]\"\n\t%1");
    }

    if(type == BRUSH_ENVMAP_EMISSION || type == MODEL_PHONG_ENVMAP_EMISSION)
    {
        vmt=vmt.arg("$detail\t\"%1\"\n\t%2").arg(pathName + "_" + SuffixNameWidget::getSuffixName(EmissiveMapName));
        vmt=vmt.arg("$detailscale\t1\n\t%1");
        vmt=vmt.arg("$detailblendmode\t5\n\t%1");
    }

    if(type < ALL_PBR && hasTint)
    {
        vmt=vmt.arg("$blendtintbymraoalpha\t1\n\t%1");
    } else if(hasTint && type == MODEL_PHONG_ENVMAP_ALPHA)
    {
        vmt=vmt.arg("$$blendtintbybasealpha\t1\n\t%1");
    }

}

void CPBRWindow::updateVMFDisplay()
{
    if(this->textureNameEdit->text().isEmpty() || !PBRAssets::getAsset(AssetType::BASE_COLOR)->operator bool() || !PBRAssets::getAsset(AssetType::ROUGHNESS_MAP)->operator bool())
    {
        QString err;
        if(!PBRAssets::getAsset(AssetType::BASE_COLOR)->operator bool())
            err += tr("Missing Base Color Texture (Required)\n\n");

        if(!PBRAssets::getAsset(AssetType::ROUGHNESS_MAP)->operator bool())
            err += tr("Missing Roughness Texture (Required)\n\n");

        if(this->textureNameEdit->text().isEmpty())
            err += tr("Missing Texture Name (Required)");

        this->vmtViewer->setText(err);

        return;
    }

    auto game = static_cast<GameType>(this->gameBox->currentData().toInt());
    auto mode = static_cast<PBRType>(this->modeBox->currentData().toInt());
    auto textureName = this->textureNameEdit->text();
//    this->getPlacementPrefix(), this->textureNameEdit->text()
    QString vmt = "%1\n{\n\t%2\n}";
    shaderType(mode,vmt);
    genericParameters(mode, game, this->getPlacementPrefix() + textureName, vmt);
    vmt = vmt.arg("");
    this->vmtViewer->setText(vmt);

}

void CPBRWindow::updateVTFDisplay()
{
//    if(!PBRAssets::getAsset(AssetType::BASE_COLOR)->operator bool() || !PBRAssets::getAsset(AssetType::ROUGHNESS_MAP)->operator bool())
//        this->infoDisplay->setText("");
    QString vtfInfo = tr("VTF Version: %1\n\n%2");
    bool isStrata = false;
    switch (static_cast<GameType>(this->gameBox->currentData().toInt())) {

        case Portal_HL2_TF2:
            vtfInfo = vtfInfo.arg("7.4");
            break;
        case Portal2_AlienSwarm:
        case GMod:
        case CSGO:
            vtfInfo = vtfInfo.arg("7.5");
            break;
        case Strata:
            isStrata = true;
            vtfInfo = vtfInfo.arg(tr("7.6\nCompression Type: ZSTD\nCompression Level: 22"));
            break;
    }

    auto mode = static_cast<PBRType>(this->modeBox->currentData().toInt());

    auto albedoAsset = PBRAssets::getAsset(AssetType::BASE_COLOR);
    if(albedoAsset->operator bool())
    {
        vtfInfo = vtfInfo.arg(tr("Base Texture:\nImage Format: %1\n\n%2"));
        vtfInfo = vtfInfo.arg(isStrata ? IMAGE_FORMATS[vtfpp::ImageFormat::STRATA_BC7] : mode < PBRType::ALL_PBR ? IMAGE_FORMATS[albedoAsset->getFormat()] : IMAGE_FORMATS[vtfpp::ImageFormat::DXT5]);
    }
    auto roughnessAsset = PBRAssets::getAsset(AssetType::ROUGHNESS_MAP);
    if(roughnessAsset->operator bool() && (mode == MODEL_PBR || mode == BRUSH_PBR))
    {
        vtfInfo = vtfInfo.arg(tr("MRAO Texture:\nImage Format: %1\n\n%2"));
        vtfInfo = vtfInfo.arg(isStrata ? IMAGE_FORMATS[vtfpp::ImageFormat::STRATA_BC7] : IMAGE_FORMATS[vtfpp::ImageFormat::DXT5]);
    }

    if(roughnessAsset->operator bool() && mode > ALL_PBR && mode < PBRType::ALL_VLG)
    {
        vtfInfo = vtfInfo.arg(tr("Phong Texture:\nImage Format: %1\n\n%2"));
        vtfInfo = vtfInfo.arg(isStrata ? IMAGE_FORMATS[vtfpp::ImageFormat::STRATA_BC7] : IMAGE_FORMATS[vtfpp::ImageFormat::DXT5]);
    }

    if(roughnessAsset->operator bool() && mode > MODEL_PHONG_ENVMAP && !(mode == BRUSH_ENVMAP || mode == MODEL_PHONG_ENVMAP))
    {
        vtfInfo = vtfInfo.arg(tr("Envmap Texture:\nImage Format: %1\n\n%2"));
        vtfInfo = vtfInfo.arg(isStrata ? IMAGE_FORMATS[vtfpp::ImageFormat::STRATA_BC7] : IMAGE_FORMATS[vtfpp::ImageFormat::DXT5]);
    }

    auto normalAsset = PBRAssets::getAsset(AssetType::NORMAL_MAP);
    if(normalAsset->operator bool())
    {
        vtfInfo = vtfInfo.arg(tr("Normal Texture:\nImage Format: %1\n\n%2"));
        vtfInfo = vtfInfo.arg(isStrata ? IMAGE_FORMATS[vtfpp::ImageFormat::STRATA_BC7] : IMAGE_FORMATS[normalAsset->getFormat()]);
    }

    auto emitAsset = PBRAssets::getAsset(AssetType::EMISSION_MAP);
    if(emitAsset->operator bool() && mode < ALL_PBR && !(mode == MODEL_PHONG_ENVMAP_EMISSION || mode == BRUSH_ENVMAP_EMISSION))
    {
        vtfInfo = vtfInfo.arg(tr("Emission Texture:\nImage Format: %1\n\n%2"));
        vtfInfo = vtfInfo.arg(isStrata ? IMAGE_FORMATS[vtfpp::ImageFormat::STRATA_BC7] : IMAGE_FORMATS[emitAsset->getFormat()]);
    }

    if(this->targetScaleBox->currentData().toInt() != -1)
    {
        vtfInfo = vtfInfo.arg(tr("Target Scale: %1\n\n%2"));
        vtfInfo = vtfInfo.arg(this->targetScaleBox->currentData().toInt());
    }

    vtfInfo = vtfInfo.arg("");
    this->infoDisplay->setText(vtfInfo);

}

bool getTextureData(const QString& path, std::vector<std::byte> &fileData, vtfpp::ImageFormat& format, int &width, int &height, int &frameCount)
{
    if(path.isEmpty())
        return false;
    format = vtfpp::ImageFormat::EMPTY;
    width = 0;
    height = 0;
    frameCount = 1;

    auto file = sourcepp::fs::readFileBuffer(path.toStdString());
    fileData = vtfpp::ImageConversion::convertFileToImageData(file, format, width, height, frameCount);
    return !fileData.empty();
}

QString CPBRWindow::getSaveDirectory()
{
    auto directory = QFileDialog::getExistingDirectory(this, QObject::tr("Save Path (relative to %1)").arg("/" + this->getPlacementPrefix()));

    if(directory.isEmpty())
        return "";

    auto basedir = QFileInfo(directory);
    if(!basedir.isWritable())
    {
        QMessageBox::critical(this, QObject::tr("No Write Permissions!"), QObject::tr("You have insufficient write permissions for %1").arg(directory));
        return "";
    }

    if(!this->getPlacementPrefix().isEmpty()) {

        auto baseDirPermissions = basedir.permissions();
        if(!QDir(directory).mkpath(this->getPlacementPrefix().removeLast(), baseDirPermissions))
        {
            QMessageBox::critical(this, QObject::tr("Unable to create subdirectory!"), QObject::tr("Unable to create subdirectory %1").arg(directory + "/" + this->getPlacementPrefix().removeLast()));
            return"";
        }

        directory += "/" + this->getPlacementPrefix();
    }

    if(!directory.endsWith('/'))
        directory.append('/');

    return directory;
}

auto remap(auto value, auto l1, auto h1, auto l2, auto h2) {
    return l2 + (value - l1) * (h2 - l2) / (h1 - l1);
}

auto remap(auto value, auto h1, auto h2) {
    return value * h2 / h1;
}

enum MathOperator : uint8_t
{
    Add = 0,
    Subtract,
    Multiply,
    Divide,
    Pow,
    Invert,
    SetR,
    SetRG,
    SetRGB,
    SetRGBA,
    SetAlpha
};

std::vector<std::byte> operateImage(std::span<const std::byte> data, float value, vtfpp::ImageFormat format, uint16_t width, uint16_t height, MathOperator op)
{
    auto tmpData = vtfpp::ImageConversion::convertImageDataToFormat(data, format, vtfpp::ImageFormat::RGBA32323232F, width, height);
    auto tmpFloatData = std::vector<float>(reinterpret_cast<float*>(tmpData.data()), reinterpret_cast<float*>(tmpData.data() + tmpData.size()));

    for(int i = 0; i < tmpFloatData.size(); i+=4)
    {

        switch (op) {
            case Add:
                tmpFloatData[i] += value;
                tmpFloatData[i + 1] += value;
                tmpFloatData[i + 2] += value;
                tmpFloatData[i + 3] += value;
                break;
            case Subtract:
                tmpFloatData[i] -= value;
                tmpFloatData[i + 1] -= value;
                tmpFloatData[i + 2] -= value;
                tmpFloatData[i + 3] -= value;
                break;
            case Multiply:
                tmpFloatData[i] *= value;
                tmpFloatData[i + 1] *= value;
                tmpFloatData[i + 2] *= value;
                tmpFloatData[i + 3] *= value;
                break;
            case Divide:
                if(tmpFloatData[i] == 0.0f || value == 0.0f ) {
                    tmpFloatData[i] = 0;
                } else
                    tmpFloatData[i] /= value;

                if(tmpFloatData[i + 1] == 0.0f || value == 0.0f ) {
                    tmpFloatData[i + 1] = 0;
                } else
                    tmpFloatData[i + 1] /= value;

                if(tmpFloatData[i + 2] == 0.0f || value == 0.0f ) {
                    tmpFloatData[i + 2] = 0;
                } else
                    tmpFloatData[i + 2] /= value;

                if(tmpFloatData[i + 3] == 0.0f || value == 0.0f ) {
                    tmpFloatData[i + 3] = 0;
                } else
                    tmpFloatData[i + 3] /= value;
                break;
            case Invert:
                tmpFloatData[i] = 1.0f - tmpFloatData[i];
                tmpFloatData[i + 1] = 1.0f - tmpFloatData[i + 1];
                tmpFloatData[i + 2] = 1.0f - tmpFloatData[i + 2];
                tmpFloatData[i + 3] = 1.0f - tmpFloatData[i + 3];
                break;
            case Pow:
                tmpFloatData[i] = std::pow(tmpFloatData[i], value);
                tmpFloatData[i + 1 ] = std::pow(tmpFloatData[i + 1], value);
                tmpFloatData[i + 2] = std::pow(tmpFloatData[i + 2], value);
                tmpFloatData[i + 3] = std::pow(tmpFloatData[i + 3], value);
                break;
            case SetRGBA:
                tmpFloatData[i + 3] = value;
            case SetRGB:
                tmpFloatData[i + 2] = value;
            case SetRG:
                tmpFloatData[i + 1] = value;
            case SetR:
                tmpFloatData[i] = value;
                break;
            case SetAlpha:
                tmpFloatData[i + 3] = value;
                break;
        }
    }

    return vtfpp::ImageConversion::convertImageDataToFormat({reinterpret_cast<std::byte*>(tmpFloatData.data()), reinterpret_cast<std::byte*>(tmpFloatData.data() + tmpFloatData.size())}, vtfpp::ImageFormat::RGBA32323232F, format, width, height);
}

std::vector<std::byte> operateImage(std::span<const std::byte> data, std::byte value, vtfpp::ImageFormat format, uint16_t width, uint16_t height, MathOperator op)
{
    return operateImage(data, remap(static_cast<float>(value), 0, 255, 0, 1), format, width, height, op);
}

std::vector<std::byte> operateImage(std::span<const std::byte> data, std::span<std::byte> value, vtfpp::ImageFormat format, uint16_t width, uint16_t height, MathOperator op)
{
    auto tmpDataA = vtfpp::ImageConversion::convertImageDataToFormat(data, format, vtfpp::ImageFormat::RGBA32323232F,width, height);
    auto tmpDataB = vtfpp::ImageConversion::convertImageDataToFormat(value, format, vtfpp::ImageFormat::RGBA32323232F,width, height);
    auto tmpFloatDataA = std::vector<float>(reinterpret_cast<float*>(tmpDataA.data()), reinterpret_cast<float*>(tmpDataA.data() + tmpDataA.size()));;

    if(op == Invert)
    {
        for(float& val : tmpFloatDataA)
            val = 1.0f - val;

        return vtfpp::ImageConversion::convertImageDataToFormat({reinterpret_cast<std::byte*>(tmpFloatDataA.data()), reinterpret_cast<std::byte*>(tmpFloatDataA.data() + tmpFloatDataA.size())}, vtfpp::ImageFormat::RGBA32323232F, format, width, height);
    }

    auto tmpFloatDataB = std::vector<float>(reinterpret_cast<float*>(tmpDataB.data()), reinterpret_cast<float*>(tmpDataB.data() + tmpDataB.size()));;

    for(int i = 0; i < tmpFloatDataA.size(); i+=4)
    {
        switch (op) {
            case Add:
                tmpFloatDataA[i + 3] += tmpFloatDataB[i + 3];
                tmpFloatDataA[i + 2] += tmpFloatDataB[i + 2];
                tmpFloatDataA[i + 1] += tmpFloatDataB[i + 1];
                tmpFloatDataA[i] += tmpFloatDataB[i];
                break;
            case Subtract:
                tmpFloatDataA[i + 3] -= tmpFloatDataB[i + 3];
                tmpFloatDataA[i + 2] -= tmpFloatDataB[i + 2];
                tmpFloatDataA[i + 1] -= tmpFloatDataB[i + 1];
                tmpFloatDataA[i] -= tmpFloatDataB[i];
                break;
            case Multiply:
                tmpFloatDataA[i + 3] *= tmpFloatDataB[i + 3];
                tmpFloatDataA[i + 2] *= tmpFloatDataB[i + 2];
                tmpFloatDataA[i + 1] *= tmpFloatDataB[i + 1];
                tmpFloatDataA[i] *= tmpFloatDataB[i];
                break;
            case Divide:
                if(tmpFloatDataA[i + 3] == 0.0f || tmpFloatDataB[i + 3] == 0.0f ) {
                    tmpFloatDataA[i + 3] = 0;
                } else
                    tmpFloatDataA[i + 3] /= tmpFloatDataB[i + 3];

                if(tmpFloatDataA[i + 2] == 0.0f || tmpFloatDataB[i + 2] == 0.0f ) {
                    tmpFloatDataA[i + 2] = 0;
                } else
                    tmpFloatDataA[i + 2] /= tmpFloatDataB[i + 2];

                if(tmpFloatDataA[i + 1] == 0.0f || tmpFloatDataB[i + 1] == 0.0f ) {
                    tmpFloatDataA[i + 1] = 0;
                } else
                    tmpFloatDataA[i + 1] /= tmpFloatDataB[i + 1];

                if(tmpFloatDataA[i] == 0.0f || tmpFloatDataB[i] == 0.0f ) {
                    tmpFloatDataA[i] = 0;
                } else
                    tmpFloatDataA[i] /= tmpFloatDataB[i];

                break;
            case Pow:
                tmpFloatDataA[i + 3] = std::pow(tmpFloatDataA[i + 3], tmpFloatDataB[i + 3]);
                tmpFloatDataA[i + 2] = std::pow(tmpFloatDataA[i + 2], tmpFloatDataB[i + 2]);
                tmpFloatDataA[i + 1] = std::pow(tmpFloatDataA[i + 1], tmpFloatDataB[i + 1]);
                tmpFloatDataA[i] = std::pow(tmpFloatDataA[i], tmpFloatDataB[i]);
                break;
            case Invert:
                break;
            case SetRGBA:
                    tmpFloatDataA[i + 3] = tmpFloatDataB[i + 3];
            case SetRGB:
                    tmpFloatDataA[i + 2] = tmpFloatDataB[i + 2];
            case SetRG:
                    tmpFloatDataA[i + 1] = tmpFloatDataB[i + 1];
            case SetR:
                    tmpFloatDataA[i] = tmpFloatDataB[i];
                break;
            case SetAlpha:
                    tmpFloatDataA[i + 3] = tmpFloatDataB[i];
                break;
        }
    }

    return vtfpp::ImageConversion::convertImageDataToFormat({reinterpret_cast<std::byte*>(tmpFloatDataA.data()), reinterpret_cast<std::byte*>(tmpFloatDataA.data() + tmpFloatDataA.size())}, vtfpp::ImageFormat::RGBA32323232F, format, width, height);
}

void savePBR(const QString& directory, const QString& namePath, const QString& name, GameType game, PBRType mode, bool isGL, int resize)
{

    uint8_t version = 4;
    uint8_t compressionLevel = 0;
    switch (game) {
        case Portal_HL2_TF2:
            break;
        case GMod:
        case Portal2_AlienSwarm:
        case CSGO:
            version = 5;
            break;
        case Strata:
            version = 6;
            compressionLevel = 22;
    }

    auto baseColorAsset = PBRAssets::getAsset(AssetType::BASE_COLOR);
    auto normalAsset = PBRAssets::getAsset(AssetType::NORMAL_MAP);
    auto heightAsset = PBRAssets::getAsset(AssetType::HEIGHT_MAP);
    auto metalAsset = PBRAssets::getAsset(AssetType::METALLIC_MAP);
    auto roughnessAsset = PBRAssets::getAsset(AssetType::ROUGHNESS_MAP);
    auto aoAsset = PBRAssets::getAsset(AssetType::AO_MAP);
    auto emissiveAsset = PBRAssets::getAsset(AssetType::EMISSION_MAP);
    auto tintAsset = PBRAssets::getAsset(AssetType::TINT_MAP);

    vtfpp::VTF::CreationOptions options;
    options.version = version;
    if(game == Strata)
    {
        options.compressionMethod = vtfpp::CompressionMethod::ZSTD;
        options.compressionLevel = compressionLevel;
    }

    options.outputFormat = game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : baseColorAsset->getFormat();

    if(resize != -1)
        options.resizeBounds = vtfpp::ImageConversion::ResizeBounds(resize);

    auto baseColorVTF = vtfpp::VTF::create(baseColorAsset->getPath().toStdString(), options);

    if(OptionsMenu::isKVDataEnabled())
        baseColorVTF.setKeyValuesDataResource(OptionsMenu::getFullMetaData().toStdString());

    baseColorVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(BaseColorName) + ".vtf").toStdString());

    if(normalAsset->operator bool())
    {
        options.invertGreenChannel = isGL;

        auto normalVTF = vtfpp::VTF::create(normalAsset->getPath().toStdString(), options);
        normalVTF.setFlags(vtfpp::VTF::FLAG_V0_NORMAL);

        if(OptionsMenu::isKVDataEnabled())
            normalVTF.setKeyValuesDataResource(OptionsMenu::getFullMetaData().toStdString());

        if(mode < ALL_PBR && heightAsset->operator bool())
        {
            normalVTF.setFormat(game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : vtfpp::ImageFormat::RGBA8888);
            int width, height, frameCount;
            vtfpp::ImageFormat format;
            std::vector<std::byte> data;
            if(getTextureData(heightAsset->getPath(), data, format, width, height, frameCount))
            {
                auto normalData = normalVTF.getImageDataAs(vtfpp::ImageFormat::RGBA8888);
                auto heightData = vtfpp::ImageConversion::convertImageDataToFormat(data, format, vtfpp::ImageFormat::I8, width, height);

                for(int i = 0, j = 0; i < heightData.size(); i++, j+=4)
                {
                    normalData[j+3] = heightData[i];
                }

                normalVTF.setImage(vtfpp::ImageConversion::convertImageDataToFormat(normalData, format, game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : vtfpp::ImageFormat::RGB888, width, height), game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : vtfpp::ImageFormat::RGB888, width, height);
            }
        }
        normalVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(NormalMapName) + ".vtf").toStdString());

        options.invertGreenChannel = false;
    }

    options.outputFormat = game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : vtfpp::ImageFormat::DXT5;
    options.initialFrameCount = 4;
    auto mraoVTF = vtfpp::VTF::create(roughnessAsset->getPath().toStdString(), options);
    if(OptionsMenu::isKVDataEnabled())
        mraoVTF.setKeyValuesDataResource(OptionsMenu::getFullMetaData().toStdString());

    mraoVTF.setImage(metalAsset->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::MITCHELL, 0, 1);
    mraoVTF.setImage(aoAsset->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::MITCHELL, 0, 2);
    mraoVTF.setImage(tintAsset->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::MITCHELL, 0, 3);

    auto roughnessData = mraoVTF.getImageDataAs(vtfpp::ImageFormat::RGBA8888,0,0);
    auto metalData = mraoVTF.getImageDataAs(vtfpp::ImageFormat::RGBA8888,0,1);
    auto aoData = mraoVTF.getImageDataAs(vtfpp::ImageFormat::RGBA8888,0,2);
    auto tintData = mraoVTF.getImageDataAs(vtfpp::ImageFormat::RGBA8888,0,3);

    bool hasAO = aoAsset->operator bool();
    bool hasTint = tintAsset->operator bool();

    for(int i = 0, j = 0; j < roughnessData.size(); i+=1, j+=4)
    {
        roughnessData[j] = metalData[j];
        //Roughness is already in place.
        if(hasAO)
            roughnessData[j + 2] = aoData[j];
        else
            roughnessData[j + 2] = std::byte(255);

        if(hasTint)
            roughnessData[j + 3] = tintData[j];
    }
    mraoVTF.setFrameCount(1);
    mraoVTF.setImage(roughnessData, vtfpp::ImageFormat::RGBA8888, mraoVTF.getWidth(), mraoVTF.getHeight(), vtfpp::ImageConversion::ResizeFilter::MITCHELL, 0, 0);
    mraoVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(MRAOName) + ".vtf").toStdString());

    if(emissiveAsset->operator bool())
    {
        options.outputFormat = game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : emissiveAsset->getFormat();
        auto emissiveVTF = vtfpp::VTF::create(emissiveAsset->getPath().toStdString(), options);
        emissiveVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(EmissiveMapName) + ".vtf").toStdString());
        if(OptionsMenu::isKVDataEnabled())
            emissiveVTF.setKeyValuesDataResource(OptionsMenu::getFullMetaData().toStdString());

    }

    auto vmtFile = QFile(directory + name + ".vmt");
    if(!vmtFile.open(QFile::WriteOnly)) {
        QMessageBox::critical(nullptr, QObject::tr("Unable to create VMT File!"), QObject::tr("Unable to create VMT File %1").arg(directory + name + ".vmt"));
        return;
    }

    QString vmt = "%1\n{\n\t%2\n}";
    shaderType(mode,vmt);
    genericParameters(mode, game, namePath + name, vmt);
    vmt = vmt.arg("");
    vmtFile.write(vmt.toLatin1());
    vmtFile.close();

}

void saveNonPBR(const QString& directory, const QString& namePath, const QString& name, GameType game, PBRType mode, bool isGL, int resize)
{
    uint8_t version = 4;
    uint8_t compressionLevel = 0;
    switch (game) {
        case Portal_HL2_TF2:
            break;
        case GMod:
        case Portal2_AlienSwarm:
        case CSGO:
            version = 5;
            break;
        case Strata:
            version = 6;
            compressionLevel = 22;
    }

    auto baseColorAsset = PBRAssets::getAsset(AssetType::BASE_COLOR);
    auto normalAsset = PBRAssets::getAsset(AssetType::NORMAL_MAP);
    auto heightAsset = PBRAssets::getAsset(AssetType::HEIGHT_MAP);
    auto metalAsset = PBRAssets::getAsset(AssetType::METALLIC_MAP);
    auto roughnessAsset = PBRAssets::getAsset(AssetType::ROUGHNESS_MAP);
    auto aoAsset = PBRAssets::getAsset(AssetType::AO_MAP);
    auto emissiveAsset = PBRAssets::getAsset(AssetType::EMISSION_MAP);
    auto tintAsset = PBRAssets::getAsset(AssetType::TINT_MAP);

    vtfpp::VTF::CreationOptions options;
    options.version = version;
    if(game == Strata)
    {
        options.compressionMethod = vtfpp::CompressionMethod::ZSTD;
        options.compressionLevel = compressionLevel;
    }
    options.outputFormat = game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : vtfpp::ImageFormat::DXT5;

    if(resize != -1)
        options.resizeBounds = vtfpp::ImageConversion::ResizeBounds(resize);

    options.initialFrameCount = 4;
    auto baseColorVTF = vtfpp::VTF::create(baseColorAsset->getPath().toStdString(), options);

    if(OptionsMenu::isKVDataEnabled())
        baseColorVTF.setKeyValuesDataResource(OptionsMenu::getFullMetaData().toStdString());

    baseColorVTF.setImage(metalAsset->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::DEFAULT, 0, 1);
    baseColorVTF.setImage(roughnessAsset->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::DEFAULT, 0, 2);
    baseColorVTF.setImage(aoAsset->getPath().toStdString(), vtfpp::ImageConversion::ResizeFilter::DEFAULT, 0, 3);

    auto rawBaseColor = baseColorVTF.getImageDataRaw();
    auto rawMetallic = baseColorVTF.getImageDataRaw(0, 1);
    auto rawRoughness = baseColorVTF.getImageDataRaw(0, 2);
    auto rawAO = baseColorVTF.getImageDataRaw(0, 3);

    std::vector<std::byte> newMetallic;

    if(!metalAsset->operator bool())
        newMetallic = operateImage(rawMetallic, 1.0f, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), SetRGBA);
    else
        newMetallic = {rawMetallic.begin(), rawMetallic.end()};

    std::vector<std::byte> newRoughness;

    if(metalAsset->operator bool())
    {
        newRoughness = operateImage(rawRoughness, 0, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Invert);
        newRoughness = operateImage(newRoughness, newMetallic, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
        newRoughness = operateImage(newRoughness, 0, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Invert);
    }
    else
    {
        newRoughness = {rawRoughness.begin(), rawRoughness.end()};
    }

    if(aoAsset->operator bool())
    {
        auto ao= operateImage(rawAO, newMetallic, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
        ao = operateImage(ao, 0.75, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
        ao = operateImage(ao, 1 - 0.75, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Add);
        newRoughness = operateImage(newRoughness, ao, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
    }

    auto alphaless = operateImage(newRoughness, 1.0f, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), SetAlpha);
    auto newBaseColor = operateImage(rawBaseColor, alphaless, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);

    baseColorVTF.setFlags(vtfpp::VTF::FLAG_V0_MULTI_BIT_ALPHA);

    auto genPhongMask = [&](){
        auto phonMask = operateImage(rawRoughness, 0, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Invert);
        phonMask = operateImage(phonMask, 3, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Pow);
        phonMask = operateImage(phonMask, 1.1, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
        if(aoAsset->operator bool())
        {
            phonMask = operateImage(phonMask, rawAO, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
        }
        return phonMask;
    };

    auto genEnvMask = [&]{
        auto roughnessExp = 3.0f;
        auto mask1 = operateImage(newMetallic, 0.75f, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
        mask1 = operateImage(mask1, 0.25, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Add);

        auto mask2 = operateImage(rawRoughness, 0, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Invert);
        mask2 = operateImage(mask2, roughnessExp, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Pow);

        if(aoAsset->operator bool())
            mask2 = operateImage(mask2, rawAO, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);

        return operateImage(mask1, mask2, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);

    };

    if(!(mode == BRUSH_ENVMAP_ALPHA || mode == MODEL_PHONG_ENVMAP_ALPHA))
    {
        bool usingPhong = game == GMod && mode == MODEL_PHONG_ENVMAP;

        if(usingPhong && (mode > ALL_PBR && mode < ALL_VLG))
        {
            auto phong = genPhongMask();
            newBaseColor = operateImage(newBaseColor, phong, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), SetAlpha);
        }
        else if (!usingPhong && mode == BRUSH_ENVMAP && mode == MODEL_PHONG_ENVMAP)
        {
            auto mask1 = genEnvMask();
            newBaseColor = operateImage(newBaseColor, mask1, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), SetAlpha);
        }
        else
            baseColorVTF.removeFlags(vtfpp::VTF::FLAG_V0_MULTI_BIT_ALPHA);

    }

    if(normalAsset->operator bool()){
        options.invertGreenChannel = isGL;
        auto normalVTF = vtfpp::VTF::create(normalAsset->getPath().toStdString(), options);

        if(OptionsMenu::isKVDataEnabled())
            normalVTF.setKeyValuesDataResource(OptionsMenu::getFullMetaData().toStdString());

        normalVTF.setSize(baseColorVTF.getWidth(), baseColorVTF.getHeight(),
                          vtfpp::ImageConversion::ResizeFilter::DEFAULT);
        normalVTF.setFlags(vtfpp::VTF::FLAG_V0_NORMAL | vtfpp::VTF::FLAG_V0_MULTI_BIT_ALPHA);
        auto rawNormalData = normalVTF.getImageDataRaw();
        std::vector<std::byte> newNormalData;

        bool usingPhong = game == GMod && mode == MODEL_PHONG_ENVMAP;

        if(usingPhong && (mode > ALL_PBR && mode < ALL_VLG))
        {
            auto phonMask = operateImage(rawRoughness, 0, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Invert);
            phonMask = operateImage(phonMask, 3, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Pow);
            phonMask = operateImage(phonMask, 1.1, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
            if(aoAsset->operator bool())
            {
                phonMask = operateImage(phonMask, rawAO, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
            }

            newNormalData = operateImage(rawNormalData, phonMask, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), SetAlpha);
            normalVTF.setImage(newNormalData, options.outputFormat, baseColorVTF.getWidth(), baseColorVTF.getHeight());
        }
        else if (!usingPhong && mode == BRUSH_ENVMAP && mode == MODEL_PHONG_ENVMAP)
        {
            auto roughnessExp = 3.0f;
            auto mask1 = operateImage(newMetallic, 0.75f, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);
            mask1 = operateImage(mask1, 0.25, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Add);

            auto mask2 = operateImage(rawRoughness, 0, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Invert);
            mask2 = operateImage(mask2, roughnessExp, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Pow);

            if(aoAsset->operator bool())
                mask2 = operateImage(mask2, rawAO, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);

            mask1 = operateImage(mask1, mask2, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), Multiply);

            newNormalData = operateImage(rawNormalData, mask1, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), SetAlpha);
            normalVTF.setImage(newNormalData, options.outputFormat, baseColorVTF.getWidth(), baseColorVTF.getHeight());
        }
        else {
            normalVTF.removeFlags(vtfpp::VTF::FLAG_V0_MULTI_BIT_ALPHA);
        }

        normalVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(NormalMapName) + ".vtf").toStdString());
        options.invertGreenChannel = false;
    }

    if(emissiveAsset->operator bool() && !(mode == MODEL_PHONG_ENVMAP_EMISSION || mode == BRUSH_ENVMAP_EMISSION))
    {
        options.outputFormat = game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : emissiveAsset->getFormat();
        auto emissiveVTF = vtfpp::VTF::create(emissiveAsset->getPath().toStdString(), options);
        if(OptionsMenu::isKVDataEnabled())
            emissiveVTF.setKeyValuesDataResource(OptionsMenu::getFullMetaData().toStdString());
        auto rawEmissive = emissiveVTF.getImageDataRaw();
        auto newEmissive = operateImage(rawEmissive, 2.2, emissiveVTF.getFormat(), emissiveVTF.getWidth(), emissiveVTF.getHeight(), Pow);
        emissiveVTF.setImage(newEmissive, options.outputFormat, emissiveVTF.getWidth(), emissiveVTF.getHeight());
        emissiveVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(EmissiveMapName) + ".vtf").toStdString());
    }

    baseColorVTF.setFrameCount(1);
    baseColorVTF.setImage(newBaseColor, options.outputFormat, baseColorVTF.getWidth(), baseColorVTF.getHeight());
    baseColorVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(BaseColorName) + ".vtf").toStdString());


    options.outputFormat = game == Strata ? vtfpp::ImageFormat::STRATA_BC7 : vtfpp::ImageFormat::DXT5;

    if(mode > ALL_PBR && mode < ALL_VLG)
    {
        auto mask = genPhongMask();
        auto phongVTF = vtfpp::VTF::create(mask, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), options);
        phongVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(PhongMapName) + ".vtf").toStdString());
    }

//    options.outputFormat

    if(mode > MODEL_PHONG_ENVMAP && !(mode == BRUSH_ENVMAP || mode == MODEL_PHONG_ENVMAP))
    {
        auto mask = genEnvMask();
        auto envVTF = vtfpp::VTF::create(mask, baseColorVTF.getFormat(), baseColorVTF.getWidth(), baseColorVTF.getHeight(), options);
        envVTF.bake((directory + name + "_" + SuffixNameWidget::getSuffixName(PhongMapName) + ".vtf").toStdString());
    }

    auto vmtFile = QFile(directory + name + ".vmt");
    if(!vmtFile.open(QFile::WriteOnly)) {
        QMessageBox::critical(nullptr, QObject::tr("Unable to create VMT File!"), QObject::tr("Unable to create VMT File %1").arg(directory + name + ".vmt"));
        return;
    }

    QString vmt = "%1\n{\n\t%2\n}";
    shaderType(mode,vmt);
    genericParameters(mode, game, namePath + name, vmt);
    vmt = vmt.arg("");
    vmtFile.write(vmt.toLatin1());
    vmtFile.close();
}

void CPBRWindow::saveTexture()
{
    auto game = static_cast<GameType>(this->gameBox->currentData().toInt());
    auto mode = static_cast<PBRType>(this->modeBox->currentData().toInt());
    auto textureName = this->textureNameEdit->text();

    if(textureName.isEmpty())
    {
        QMessageBox::critical(this, tr("Texture Name must be specified!"), tr("The VMT/texture name must be specified."));
        return;
    }

    if(!PBRAssets::getAsset(BASE_COLOR)->operator bool())
    {
        QMessageBox::critical(this, tr("Base Color texture must be specified!"), tr("The Base Color texture ($basetexture) must be specified."));
        return;
    }

    if(!PBRAssets::getAsset(ROUGHNESS_MAP)->operator bool())
    {
        QMessageBox::critical(this, tr("Roughness texture must be specified!"), tr("The Roughness texture must be specified."));
        return;
    }

    if(mode < ALL_PBR && game != Strata)
    {
        QMessageBox::warning(this, tr("Incompatible Game."), tr("The PBR shader is only supported on Strata Source games.\nThe textures/VMT will be generated but is unsupported for the currently selected game branch."));
    }

    auto saveDir = this->getSaveDirectory();

    if(saveDir.isEmpty())
        return;

    if(mode < ALL_PBR)
        savePBR(saveDir, this->getPlacementPrefix(), this->textureNameEdit->text(), game, mode, this->normalBox->currentData().toBool(), this->targetScaleBox->currentData().toInt());
    else
        saveNonPBR(saveDir, this->getPlacementPrefix(), this->textureNameEdit->text(), game, mode, this->normalBox->currentData().toBool(), this->targetScaleBox->currentData().toInt());
}

void CPBRWindow::timerEvent(QTimerEvent *event) {
    if(!this->isDirty())
        return QObject::timerEvent(event);

    this->updateVMFDisplay();
    this->updateVTFDisplay();

    saveButton->setDisabled(this->textureNameEdit->text().isEmpty() || !PBRAssets::getAsset(BASE_COLOR)->operator bool() || !PBRAssets::getAsset(ROUGHNESS_MAP)->operator bool());

    this->unmarkDirty();
    QObject::timerEvent(event);
}

PBRAssets::PBRAssets(QWidget *parent, AssetType id, const QString &display, bool isRequired) : QWidget(parent)
{
    this->setAcceptDrops(true);

    auto layout = new QGridLayout(this);
    this->display = new QLabel(this);
    this->display->setFrameShape(QFrame::Panel);
    this->display->setFrameShadow(QFrame::Sunken);
    this->display->setLineWidth(1);
    this->display->setMidLineWidth(2);

    applyDisplaySettings(this->display);
    layout->addWidget(this->display,0,0,2,1);
    auto typeLayout = new QHBoxLayout();
    this->type = new QLabel(display,this);
    typeLayout->addWidget(this->type);

    if(isRequired) {
        auto requiredLabel = new QLabel(tr("(Required)"),this);
        requiredLabel->setDisabled(true);
        typeLayout->addWidget(requiredLabel);
    }

    layout->addLayout(typeLayout,0,1);
    this->path = new QPathLineEdit(this);
    this->path->setReadOnly(true);
    connect(this->path, &QPathLineEdit::onRightClicked, this, [&]{
        this->display->setPixmap(getUploadImage());
        this->path->clear();
        this->valid = false;
        this->format = vtfpp::ImageFormat::EMPTY;
        emit this->assetChanged();
    });
    connect(this->path, &QPathLineEdit::onDoubleClicked, this, &PBRAssets::setNewImageByDialog);
    layout->addWidget(this->path,1,1);

    this->isRequired = isRequired;
    PBRAssets::assets.insert(id, this);
}

bool PBRAssets::setNewImageByDialog()
{
    auto fileDialog =  QFileDialog::getOpenFileName(this, tr("Get Image"), lastOpened, supported_images);
    if(fileDialog.isEmpty())
        return false;

    int width, height, frameCount;
    vtfpp::ImageFormat fmt;
    auto file = sourcepp::fs::readFileBuffer(fileDialog.toStdString());
    auto fileData = vtfpp::ImageConversion::convertFileToImageData(file, fmt, width, height, frameCount);
    if(fileData.empty())
        return false;

    this->path->setText(fileDialog);
    this->format = fmt;

    auto imageData = vtfpp::ImageConversion::convertImageDataToFormat(fileData, format, vtfpp::ImageFormat::RGBA8888, width, height);

    this->display->setPixmap(QPixmap::fromImage(QImage(reinterpret_cast<const unsigned char*>(imageData.data()), width, height, QImage::Format_RGBA8888)).scaled(64,64));
    this->valid = true;
    emit this->assetChanged();
    return true;
}

void PBRAssets::mouseDoubleClickEvent(QMouseEvent *event) {

    if(!this->setNewImageByDialog())
        return event->ignore();
    return event->accept();
}

void PBRAssets::dragEnterEvent(QDragEnterEvent *event) {
    if(!event->mimeData()->hasUrls())
        return event->ignore();

    if(event->mimeData()->urls().isEmpty())
        return event->ignore();

    static QStringList supported = QString(supported_images).replace('*',"").split(" ");

    bool foundSupported = false;
    auto url = event->mimeData()->urls()[0];
    for(const auto & fmt : supported)
    {
        if(url.fileName().endsWith(fmt))
        {
            foundSupported = true;
            break;
        }
    }

    if(!foundSupported)
        return event->ignore();

    return event->acceptProposedAction();

}

void PBRAssets::dropEvent(QDropEvent *event) {

    if(!event->mimeData()->hasUrls())
        return event->ignore();
    auto url = event->mimeData()->urls()[0].toLocalFile();


    auto file = sourcepp::fs::readFileBuffer(url.toStdString());
    int width, height, frameCount;
    vtfpp::ImageFormat fmt;
    auto fileData = vtfpp::ImageConversion::convertFileToImageData(file, fmt, width, height, frameCount);

    if(fileData.empty())
        return;

    this->format = fmt;

    this->path->setText(url);

    auto imageData = vtfpp::ImageConversion::convertImageDataToFormat(fileData, format, vtfpp::ImageFormat::RGBA8888, width, height);

    this->display->setPixmap(QPixmap::fromImage(QImage(reinterpret_cast<const unsigned char*>(imageData.data()), width, height, QImage::Format_RGBA8888)).scaled(64,64));
    this->valid = true;
    emit this->assetChanged();
    event->accept();
}

void PBRAssets::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton)
    {
        this->display->setPixmap(getUploadImage());
        this->path->clear();
        event->accept();
        this->valid = false;
        this->format = vtfpp::ImageFormat::EMPTY;
        emit this->assetChanged();
        return;
    }
    QWidget::mousePressEvent(event);
}

QString PBRAssets::getPath() const {
    return this->path->text();
}

bool isNewLine(char c) {
    return c == '\n' || c == '\r';
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\a' || c == '\f' || c == '\t' || c == '\v' || isNewLine(c);
}

#include "mainwindow.moc"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) 
{
    this->typeFormat.setForeground(Qt::cyan);
    this->braceFormat.setForeground(Qt::yellow);
    this->keywordFormat.setForeground(Qt::lightGray);
    this->stringFormat.setForeground(Qt::green);
    this->valueFormat.setForeground(QColor(0xFF69B4));
}
int currentBlock = 0;
void Highlighter::highlightBlock(const QString &text)
{
    if(text.isEmpty())
        return;

    if(text == '{' || text == '}')
        return this->setFormat(0,1, this->braceFormat);

    if(text == "PBR" || text == "VertexLitGeneric" || text == "LightmappedGeneric")
        return this->setFormat(0, text.length(), this->typeFormat);

    int length = 0;
    QChar chr = text[length];
    while(isWhitespace(chr.toLatin1()))
    {
        length++;
        if(text.length() == length)
            return;
        chr = text[length];
    }

    int start = length;

    if(chr == '"')
    {
        length++;
        if(text.length() == length)
            return;
        chr = text[length];
        while(chr != '"')
        {
            length++;
            if(text.length() == length)
                return;
            chr = text[length];
        }

        this->setFormat(start, length, this->stringFormat);
        length++;
        if(text.length() == length)
            return;
        chr = text[length];

    } else
    {
        while(!isWhitespace(chr.toLatin1()))
        {
            length++;
            if(text.length() == length)
                return;
            chr = text[length];
        }
        this->setFormat(start, length, this->keywordFormat);
    }

    while(isWhitespace(chr.toLatin1()))
    {
        length++;
        if(text.length() == length)
            return;
        chr = text[length];
    }

    start = length;

    if(chr == '"')
    {
        length++;
        if(text.length() == length)
            return;
        chr = text[length];
        while(chr != '"')
        {
            length++;
            if(text.length() == length)
                return;
            chr = text[length];
        }

        this->setFormat(start, length, this->stringFormat);

    } else
    {
        while(!isWhitespace(chr.toLatin1()))
        {
            length++;
            if(text.length() == length)
                break;
            chr = text[length];
        }
        this->setFormat(start, length, this->valueFormat);
    }
}

QString getDefaultTextureNames(SuffixNames name)
{
    switch (name) {
        case BaseColorName:
            return baseColor;
        case NormalMapName:
            return normalMap;
        case EmissiveMapName:
            return emissionMask;
        case PhongMapName:
            return phongMask;
        case EnvMapName:
            return envMask;
        case MRAOName:
            return metalRoughnessAO;
        case TintMapName:
            return tintMap;
    }
    return "";
}

void OptionsMenu::setOptionsAndDefaults()
{
    for(auto [key, value] : SuffixNameWidget::getSuffixNameWidgets().asKeyValueRange())
    {
        value->setCurrentTextureName("");
        auto res = this->settings->value(getDefaultTextureNames(key));
        if(SuffixNameWidget::getSuffixName(key) != res.toString())
            value->setCurrentTextureName(res.toString());
    }

    for(auto [key, value] : OptionsMenu::vtfMetaData.asKeyValueRange())
    {
        value->setText(this->settings->value(key, "").toString());
    }

    setKVEnable(this->settings->value("MetadataEnabled", true).toBool());
}

OptionsMenu::OptionsMenu(QWidget *parent) : QDialog(parent)
{
    this->settings = new QSettings("settings.ini", QSettings::IniFormat, this);

    auto layout = new QGridLayout(this);

    auto suffixGroupBox = new QGroupBox(tr("File Suffixes"),this);
    auto suffixLayout = new QVBoxLayout(suffixGroupBox);
    suffixLayout->addWidget(new SuffixNameWidget(this, BaseColorName, tr("Base Color")));
    suffixLayout->addWidget(new SuffixNameWidget(this, NormalMapName, tr("Normal Map")));
    suffixLayout->addWidget(new SuffixNameWidget(this, EmissiveMapName, tr("Emissive Map")));
    suffixLayout->addWidget(new SuffixNameWidget(this, PhongMapName, tr("Phong Map")));
    suffixLayout->addWidget(new SuffixNameWidget(this, EnvMapName, tr("Env Map")));
    suffixLayout->addWidget(new SuffixNameWidget(this, MRAOName, tr("MRAO Texture")));
    suffixLayout->addWidget(new SuffixNameWidget(this, TintMapName, tr("Tint Map")));
    layout->addWidget(suffixGroupBox,0,0);

    auto metaDataGroupBox = new QGroupBox(tr("Meta Data"),this);
    metaDataGroupBox->setCheckable(true);
    auto metaDataLayout = new QGridLayout(metaDataGroupBox);

    metaDataLayout->addWidget(new QLabel(tr("Author:"), this), 0, 0);
    auto authorLineEdit = new QLineEdit(this);
    OptionsMenu::vtfMetaData["Author"] = authorLineEdit;
    metaDataLayout->addWidget(authorLineEdit, 0, 1);
    metaDataLayout->addWidget(new QLabel(tr("Contact:"), this), 1, 0);
    auto contactLineEdit = new QLineEdit(this);
    OptionsMenu::vtfMetaData["Contact"] = contactLineEdit;
    metaDataLayout->addWidget(contactLineEdit, 1, 1);
    metaDataLayout->addWidget(new QLabel(tr("Organization:"), this), 2, 0);
    auto organizationLineEdit = new QLineEdit(this);
    OptionsMenu::vtfMetaData["Organization"] = organizationLineEdit;
    metaDataLayout->addWidget(organizationLineEdit, 2, 1);
    metaDataLayout->addWidget(new QLabel(tr("Version:"), this), 3, 0);
    auto versionLineEdit = new QLineEdit(this);
    OptionsMenu::vtfMetaData["Version"] = versionLineEdit;
    metaDataLayout->addWidget(versionLineEdit, 3, 1);
    metaDataLayout->addWidget(new QLabel(tr("Modification:"), this), 4, 0);
    auto modificationLineEdit = new QLineEdit(this);
    OptionsMenu::vtfMetaData["Modification"] = modificationLineEdit;
    metaDataLayout->addWidget(modificationLineEdit, 4, 1);
    metaDataLayout->addWidget(new QLabel(tr("Description:"), this), 5, 0);
    auto descriptionLineEdit = new QLineEdit(this);
    OptionsMenu::vtfMetaData["Description"] = descriptionLineEdit;
    metaDataLayout->addWidget(descriptionLineEdit, 5, 1);
    metaDataLayout->addWidget(new QLabel(tr("Comments:"), this), 6, 0);
    auto commentLineEdit = new QLineEdit(this);
    OptionsMenu::vtfMetaData["Comment"] = commentLineEdit;
    metaDataLayout->addWidget(commentLineEdit, 6, 1);
    metaDataLayout->addWidget(new QLabel(tr("Will also log creation time."), this), 7, 0, 1, 2);

    layout->addWidget(metaDataGroupBox,0,1);

    auto dialogButtons = new QDialogButtonBox(this);
    dialogButtons->addButton(tr("Apply"), QDialogButtonBox::AcceptRole);
    dialogButtons->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    layout->addWidget(dialogButtons,1,0, 1, 2);

    this->setOptionsAndDefaults();

    connect(metaDataGroupBox, &QGroupBox::clicked, this, &OptionsMenu::setKVEnable);
    connect(dialogButtons, &QDialogButtonBox::accepted, this, &OptionsMenu::accept);
    connect(dialogButtons, &QDialogButtonBox::rejected, this, &OptionsMenu::reject);

    connect(this, &OptionsMenu::accepted, this, [&]{
        for(auto [key, value] : SuffixNameWidget::getSuffixNameWidgets().asKeyValueRange())
        {
            this->settings->setValue(getDefaultTextureNames(key), value->getCurrentSuffixName());
        }

        for(auto [key, value] : OptionsMenu::vtfMetaData.asKeyValueRange())
        {
            this->settings->setValue(key, value->text());
        }

        this->settings->setValue("MetadataEnabled", OptionsMenu::isKVDataEnabled());

    });
    connect(this, &OptionsMenu::rejected, this, &OptionsMenu::setOptionsAndDefaults);

}

QString OptionsMenu::getFullMetaData() {
    QString metadata = "Information\n{\n\"%1\"\t\"%2\"\n%3\n}";
    for(auto [key, value] : OptionsMenu::vtfMetaData.asKeyValueRange())
    {
        if(value->text().isEmpty())
            continue;
        metadata = metadata.arg(key, value->text(), "\n\"%1\"\t\"%2\"\n%3");
    }
    metadata = metadata.arg("CreatedOn", QString::number(QDateTime::currentMSecsSinceEpoch()), "\n\"%1\"\t\"%2\"\n%3");
    metadata = metadata.arg("LastModified", QString::number(QDateTime::currentMSecsSinceEpoch()), "");

    return metadata;
}

bool OptionsMenu::isKVDataEnabled()
{
    return OptionsMenu::isEnabled;
}

SuffixNameWidget::SuffixNameWidget(QWidget *parent, SuffixNames nameType, const QString &displayName)
        : QWidget(parent), suffixNameType(nameType)
{
    auto layout = new QHBoxLayout(this);

    layout->addWidget(new QLabel(tr("Output %1 suffix:").arg(displayName), this));
    this->suffixName = new QLineEdit(this);
    this->suffixName->setPlaceholderText(getDefaultTextureNames(nameType));
    layout->addWidget(this->suffixName);
    auto defaultButton = new QPushButton(this);
    defaultButton->setIcon(this->style()->standardIcon(QStyle::StandardPixmap::SP_BrowserReload));
    defaultButton->setIconSize({this->suffixName->height() / 2, this->suffixName->height() / 2});
    layout->addWidget(defaultButton);

    connect(defaultButton, &QPushButton::clicked, this->suffixName, &QLineEdit::clear);

    SuffixNameWidget::suffixNameWidgets[nameType] = this;
}

void SuffixNameWidget::setCurrentTextureName(const QString &name)
{
    this->suffixName->setText(name);
}

QString SuffixNameWidget::getCurrentSuffixName() {

    if(this->suffixName->text().isEmpty())
        return getDefaultTextureNames(suffixNameType);

    return this->suffixName->text().replace(' ', '_');
}
