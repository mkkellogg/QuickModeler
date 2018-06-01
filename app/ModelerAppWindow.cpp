#include "ModelerAppWindow.h"

namespace Modeler {
    ModelerAppWindow::ModelerAppWindow() {

    }

    ModelerAppWindow::~ModelerAppWindow() {

    }

    bool ModelerAppWindow::initialize(ModelerApp* modelerApp) {
        this->modelerApp = modelerApp;
        return true;
    }

}

