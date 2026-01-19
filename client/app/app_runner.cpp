#include "context/client_context.hpp"

int run_client(int argc, char** argv) {
    std::cout << "[rtype_client] Bootstrapping..." << std::endl;
    ClientContext ctx;
    initialize_context(ctx, argc, argv);
    while (ctx.window.isOpen()) {
        if (!handle_events(ctx)) break;
        if (!process_frame(ctx)) break;
    }
    if (ctx.net_client) ctx.net_client->shutdown();
    std::cout << "[rtype_client] Client shutting down." << std::endl;
    return 0;
}
