#include <QApplication>
#include <QCoreApplication>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <string_view>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("VLiva Public Repository Notice"));

    QWidget window;
    window.setWindowTitle(QStringLiteral("VLiva — Public Repository Notice"));
    window.setMinimumSize(720, 440);
    window.setStyleSheet(QStringLiteral(
        "QWidget { background: #10131a; color: #e9edf5; }"
        "QLabel#brand { color: #8dc7ff; font-size: 34px; font-weight: 700; }"
        "QLabel#heading { font-size: 22px; font-weight: 650; }"
        "QLabel#body { color: #bdc7d8; font-size: 15px; line-height: 1.4; }"
        "QLabel#footer { color: #7f8ba0; font-size: 13px; }"
        "QLabel a { color: #73b7ff; }"));

    auto* layout = new QVBoxLayout(&window);
    layout->setContentsMargins(64, 52, 64, 48);
    layout->setSpacing(20);

    auto* brand = new QLabel(QStringLiteral("VLiva"), &window);
    brand->setObjectName(QStringLiteral("brand"));
    layout->addWidget(brand);

    auto* heading = new QLabel(
        QStringLiteral("This is not the source code for the proprietary VLiva application."),
        &window);
    heading->setObjectName(QStringLiteral("heading"));
    heading->setWordWrap(true);
    layout->addWidget(heading);

    auto* body = new QLabel(
        QStringLiteral(
            "This public repository is the project hub and contains selected public interfaces, "
            "documentation, and independently licensed components. Building this target only "
            "creates this informational notice window; it does not build the commercial VLiva "
            "desktop application.<br><br>"
            "The complete standalone OBS source plugin is available under <b>plugins/obs</b> "
            "under GPL-2.0-or-later. The public plugin C ABI is available under <b>include/vliva</b>."),
        &window);
    body->setObjectName(QStringLiteral("body"));
    body->setTextFormat(Qt::RichText);
    body->setWordWrap(true);
    body->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(body);
    layout->addStretch();

    auto* footer = new QLabel(
        QStringLiteral(
            "Official information: <a href=\"https://vliva.tamkungz.me/\">vliva.tamkungz.me</a> · "
            "Issues: <a href=\"https://github.com/TamKungZ/VLiva/issues\">GitHub</a>"),
        &window);
    footer->setObjectName(QStringLiteral("footer"));
    footer->setOpenExternalLinks(true);
    layout->addWidget(footer);

    window.show();

    const bool smokeTest = std::any_of(
        argv + 1,
        argv + argc,
        [](const char* argument) { return std::string_view(argument) == "--smoke-test"; });
    if (smokeTest) {
        QTimer::singleShot(100, &application, &QCoreApplication::quit);
    }

    return application.exec();
}
