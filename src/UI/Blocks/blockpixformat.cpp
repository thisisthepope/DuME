#include "blockpixformat.h"

BlockPixFormat::BlockPixFormat(MediaInfo *mediaInfo, QWidget *parent) :
    BlockContentWidget(mediaInfo,parent)
{
#ifdef QT_DEBUG
    qDebug() << "Create PixFormat block";
#endif
    setupUi(this);
    _freezeUI = true;
    listPixFormats();
    _freezeUI = false;
}

void BlockPixFormat::activate(bool activate)
{
    _freezeUI = true;

    if (activate && pixFmtFilterBox->currentIndex() != 0 )
    {
        _mediaInfo->setPixFormat( pixFmtBox->currentData(Qt::UserRole).toString() );
    }
    else
    {
        _mediaInfo->setPixFormat( "" );
    }

    _freezeUI = false;
}

void BlockPixFormat::update()
{
    qDebug() << "Update Pix format Block";
    if (_freezeUI) return;

    if (!_mediaInfo->hasVideo())
    {
        emit blockEnabled(false);
        _freezeUI = false;
        return;
    }

    VideoInfo *stream = _mediaInfo->videoStreams()[0];
    if(stream->isCopy())
    {
        emit blockEnabled(false);
        _freezeUI = false;
        return;
    }
    emit blockEnabled(true);

    listPixFormats( );

    if (_pixFormats.count() == 0)
    {
        emit blockEnabled(false);
        _freezeUI = false;
        return;
    }

    _freezeUI = true;

    filterPixFormats(false);

    _freezeUI = true;

    FFPixFormat *pf = stream->pixFormat();
    setPixFormat( pf->name() );

    _freezeUI = false;
    qDebug() << "Pix format Block updated";
}

void BlockPixFormat::listPixFormats()
{
    _freezeUI = true;

    QString prevFilter = pixFmtFilterBox->currentText();
    QString prevFormat = pixFmtBox->currentData().toString();
    pixFmtBox->clear();
    pixFmtFilterBox->clear();
    _pixFormats.clear();

    if (!_mediaInfo->hasVideo())
    {
        _freezeUI = false;
        return;
    }

    FFCodec *vc = _mediaInfo->videoStreams()[0]->codec();
    if ( vc->name() == "" ) vc = _mediaInfo->defaultVideoCodec();

    _pixFormats = vc->pixFormats();

    QStringList filters = QStringList();
    foreach( FFPixFormat *p, vc->pixFormats() )
    {
        QString filter = QString::number(p->bitsPerPixel()) + " bits - " + QString::number(p->numComponents()) + " channels" ;
        if (!filters.contains(filter)) filters << filter;
    }

    filters.sort();
    pixFmtFilterBox->addItem("Default");
    pixFmtFilterBox->addItem("All bit depths");
    foreach(QString filter,filters)
    {
        pixFmtFilterBox->addItem(filter);
    }

    if (_pixFormats.count() == 0)
    {
        _freezeUI = false;
        return;
    }

    // reselect previous
    setFilter( prevFilter );
    setPixFormat( prevFormat );

    _freezeUI = false;
}

void BlockPixFormat::filterPixFormats(bool resetPrevious)
{
    _freezeUI = true;

    QString prevFormat = pixFmtBox->currentData().toString();
    pixFmtBox->clear();

    bool alpha = _mediaInfo->hasAlpha();
    QString filter = pixFmtFilterBox->currentText();
    foreach( FFPixFormat *p, _pixFormats )
    {
        if ( p->hasAlpha() == alpha && ( p->prettyName().indexOf( filter ) > 0 || pixFmtFilterBox->currentIndex() <= 1 ) )
        {
            pixFmtBox->addItem(p->prettyName(), p->name() );
        }
    }

    if (resetPrevious) setPixFormat( prevFormat, false );

    _freezeUI = false;
}

void BlockPixFormat::setDefaultPixFormat()
{
    _freezeUI = true;

    pixFmtFilterBox->setCurrentIndex( 0 );
    filterPixFormats(false);

    _freezeUI = true;

    if (!_mediaInfo->hasVideo())
    {
        _freezeUI = false;
        return;
    }

    FFPixFormat *pf = _mediaInfo->videoStreams()[0]->defaultPixFormat();
    if (pf->name() == "") pf = _mediaInfo->defaultPixFormat();

    if (pf->name() == "")
    {
        pixFmtBox->setCurrentIndex( -1 );
    }
    else
    {
        setPixFormat( pf->name(), false );
    }

    pixFmtBox->setEnabled( false );

    _freezeUI = false;
}

void BlockPixFormat::setPixFormat(QString name, bool tryWithoutFilter )
{
    _freezeUI = true;

    if (name == "" && _pixFormats.count() > 0)
    {
        setDefaultPixFormat();
        _freezeUI = false;
        return;
    }

    pixFmtBox->setCurrentData(name);
    if (pixFmtBox->currentIndex() >= 0)
    {
        _freezeUI = false;
        return;
    }

    //try again without filter
    if ( tryWithoutFilter )
    {
        pixFmtFilterBox->setCurrentIndex( 1 );
        filterPixFormats();
        setPixFormat( name, false );
    }
    else
    {
        pixFmtBox->setCurrentIndex( -1 );
    }

    _freezeUI = false;
}

void BlockPixFormat::setFilter(QString name)
{
    _freezeUI = true;

    pixFmtFilterBox->setCurrentText( name );
    filterPixFormats();

    _freezeUI = false;
}

void BlockPixFormat::on_pixFmtFilterBox_currentIndexChanged(int index)
{
    if (_freezeUI) return;
    if ( index == 0 )
    {
        _mediaInfo->setPixFormat( "" );
    }
    else
    {
        filterPixFormats();
        pixFmtBox->setEnabled(true);
    }

}

void BlockPixFormat::on_pixFmtBox_currentIndexChanged(int index)
{
    if (_freezeUI) return;
    _mediaInfo->setPixFormat( pixFmtBox->itemData(index, Qt::UserRole).toString() );
}
