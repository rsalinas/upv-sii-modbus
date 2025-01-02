#include "aboutdialog.h"
#include <QDesktopServices>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About");

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Load the image (PNG format is recommended)
    imageLabel = new QLabel(this);
    QPixmap pixmap(":/images/logo.png"); // Ensure the image is added to your resource file (.qrc)
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel);

    // Text with authors, license, and URL
    textLabel = new QLabel(this);
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setText("<h2>About</h2>"
                       "<p>Authors: Manuel and Raúl</p>"
                       "<p>License: Copyleft</p>"
                       "<p>Visit: <a href=\"http://www.disca.upv.es\">www.disca.upv.es</a></p>");
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    textLabel->setOpenExternalLinks(true);
    textLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(textLabel);

    // Button to close the dialog
    closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &AboutDialog::accept);
    layout->addWidget(closeButton);

    setLayout(layout);
}
