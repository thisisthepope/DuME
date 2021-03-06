#ifndef BLOCKALPHA_H
#define BLOCKALPHA_H

#include "ui_blockalpha.h"
#include "UI/Blocks/blockcontentwidget.h"

class BlockAlpha : public BlockContentWidget, private Ui::BlockAlpha
{
    Q_OBJECT

public:
    explicit BlockAlpha(MediaInfo *mediaInfo, QWidget *parent = nullptr);
public slots:
    void activate( bool blockEnabled );
    void update();
private slots:
    void on_alphaButton_clicked(bool checked);
    void on_unmultButton_clicked(bool checked);
};

#endif // BLOCKALPHA_H
