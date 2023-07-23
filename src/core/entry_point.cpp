#include "entry_point.h"

#include "core/application.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[]) {
    Phos::Application* application = create_application();
    application->run();
    delete application;

    return 0;
}
