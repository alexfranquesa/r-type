# engine/render – Rendering Helpers

Purpose
-------
`engine/render` provides small utilities on top of SFML (texture loading, sprite bank). It does not contain gameplay logic.

Key files
---------
- `engine/render/texture_loader.hpp`: load textures from disk.
- `engine/render/sprite_bank.hpp`: shared sprite registry used by the client renderer.
- `engine/render/render_helpers.hpp`: helper functions for drawing.
- `engine/render/renderer_interface.hpp`: `IRenderer` interface + `SpriteView` used by client glue.

Rules
-----
- SFML is allowed here.
- No gameplay or networking logic inside render helpers.
- Gameplay code and the client glue should talk to `IRenderer`, not raw SFML.
