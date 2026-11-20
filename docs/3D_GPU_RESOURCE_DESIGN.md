# 3D GPU Mesh Resource Design

## Status

- **Design status:** defined against the current Android/Vulkan implementation.
- **Target branch:** `feature/3d-renderer-foundation`.
- **Current base:** `f8dd2371243aa00dd5cce83a3e13288ba2ab6fdf`.
- **Implemented elsewhere:** CPU-side `TgMesh3D`, validation, XZ plane writer, and `TgCamera3D`.
- **Not implemented by this document:** Vulkan mesh buffers, upload code, a 3D pipeline, depth attachment, or a visible 3D scene.

This is an implementation contract, not evidence that a GPU upload or 3D draw has already run.

## 1. Findings from the current Vulkan path

The current Android renderer in `src/main.c` is still a single Vulkan owner and frame loop:

- Device creation requests Vulkan 1.0 and selects one queue family that supports both graphics and presentation.
- The current render pass has a color attachment only. There is no depth image or depth attachment yet.
- The existing 2D path creates one large host-visible, host-coherent vertex buffer and rewrites it each frame.
- Texture creation already demonstrates a staging-buffer upload into device-local image memory. That code is embedded in `main.c` and waits with `vkQueueWaitIdle`; it is a reference for the basic transfer route, not a reusable resource layer.
- `closevk()` waits for the device to become idle and destroys the device-scoped resources. `APP_CMD_TERM_WINDOW` currently performs a full Vulkan teardown, so device resources must be recreated after the next window initialization.
- `build.sh` compiles `src/*.c`, but shader source compilation is explicit and currently lists the two 2D menu shaders.

The 3D mesh path must not assume that the current 2D buffer, shader inputs, render pass, or pipeline can render 3D geometry.

## 2. Ownership contract

| Layer | Owns | Must not own |
|---|---|---|
| Sea scene / gameplay | Canonical CPU mesh data, dimensions, material parameters | Vulkan handles, device memory, queue submission |
| 3D renderer | Logical mesh reference, camera, draw metadata, 3D pipeline state | Allocation/free of `VkDeviceMemory`, swapchain acquire/present |
| Shared Vulkan backend | `VkBuffer`, `VkDeviceMemory`, memory-type selection, upload commands, upload fences, device-scoped destruction | Sea-specific geometry or simulation |
| Frame coordinator | Frame command buffer, frame fences/semaphores, submit and present | Mesh CPU arrays or scene-specific policy |

The renderer may receive a **borrowed** `VkBuffer` handle or a backend-controlled bind operation while recording a frame. It never receives ownership of the buffer's allocation, may not free it, and may not keep using a borrowed command buffer outside the active frame.

The CPU mesh remains the canonical recoverable source. A GPU mesh is a device-scoped derivative of that source.

## 3. Stage-1 buffer layout

The first sea mesh is static, position-only, and tiny (four vertices and six 32-bit indices). For this case, use one combined GPU buffer rather than separate vertex and index allocations.

Calculate sizes with checked `VkDeviceSize` arithmetic:

```text
vertexOffset = 0
vertexBytes  = vertexCount * sizeof(TgMeshVertex3D)
indexOffset  = alignUp(vertexBytes, sizeof(uint32_t))
indexBytes   = indexCount * sizeof(uint32_t)
totalBytes   = indexOffset + indexBytes
```

Required alignment is four bytes because the index data is `uint32_t`. Check every multiplication, alignment addition, and final addition for overflow before creating Vulkan objects.

Create the destination buffer with:

- `VK_BUFFER_USAGE_TRANSFER_DST_BIT`
- `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`
- `VK_BUFFER_USAGE_INDEX_BUFFER_BIT`

The vertex binding uses offset `vertexOffset` and stride `sizeof(TgMeshVertex3D)`. The index binding uses `indexOffset` and `VK_INDEX_TYPE_UINT32`. Draw with `vkCmdDrawIndexed`.

This combined layout reduces allocations and simplifies the first upload. It does not prevent a later split-buffer strategy if dynamic updates or different vertex streams prove useful.

## 4. Memory selection and upload route

### Destination

Prefer a memory type allowed by `vkGetBufferMemoryRequirements` that has `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`. The backend must inspect the actual device memory properties; it must not assume that a particular memory type index exists.

If no compatible device-local type exists, allow a documented host-visible fallback only when the selected type satisfies the buffer's requirements. Record which path was selected so device testing can distinguish staged from directly mapped memory.

### Staging

For the normal path, create one temporary staging buffer with `VK_BUFFER_USAGE_TRANSFER_SRC_BIT`. Its memory must be host visible; prefer host coherent memory when available.

When the selected staging memory is not host coherent:

- Map the allocation.
- Copy the CPU vertex and index bytes into their calculated offsets.
- Flush the written range with `vkFlushMappedMemoryRanges`.
- Respect `nonCoherentAtomSize` for sub-ranges. A whole-allocation mapped range may be flushed as a whole when its range satisfies Vulkan's requirements.

Do not silently treat all host-visible memory as coherent.

### Upload command

For the first static mesh:

1. Validate the CPU mesh again at the upload boundary.
2. Compute and validate the combined buffer layout.
3. Create the destination and staging buffers; allocate and bind their memory.
4. Map staging memory, pack vertex/index data, flush when necessary, then unmap.
5. Allocate a short-lived command buffer from a transient upload command pool for the existing graphics queue family.
6. Begin with `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`.
7. Record two `vkCmdCopyBuffer` regions: vertices and indices.
8. Record a `VkBufferMemoryBarrier` for the destination buffer from `VK_ACCESS_TRANSFER_WRITE_BIT` to `VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT`, with transfer as the source stage and vertex-input as the destination stage.
9. End and submit on the same graphics queue using a dedicated upload fence.
10. Wait for that fence, then free the temporary command buffer/pool and staging buffer/memory. Keep the destination buffer/memory alive for rendering.

Use a fence for this Vulkan 1.0 path. Do not require timeline semaphores or a dedicated transfer queue for the first experiment. Do not use `vkQueueWaitIdle` for every mesh upload; wait for the upload's own fence instead.

The current graphics queue family is used for both copy and draw, so no queue-family ownership transfer is needed. A future dedicated transfer queue would require explicit semaphore ordering and, for exclusive resources crossing queue families, queue-family ownership transfers.

For the rare direct-mapped destination fallback, the backend must also make host writes visible to vertex/index reads with the appropriate host-to-device memory dependency before drawing.

## 5. Lifetime and recreation rules

### Successful upload

- The upload call copies from the CPU arrays and waits for its upload fence before returning.
- After it returns, the temporary staging allocation is no longer needed.
- The CPU arrays remain owned by the sea scene, not by the GPU mesh.
- The destination GPU buffer remains valid until explicitly destroyed or until its `VkDevice` is destroyed.

### Destruction

- Do not destroy a mesh while a submitted frame may still reference it.
- For the initial single-frame-in-flight path, wait for the relevant frame fence or use the existing device-idle teardown before destruction.
- Destroy the `VkBuffer` before freeing the `VkDeviceMemory` bound to it.
- Destruction must be safe after partial initialization: check each handle and release only successfully created objects.
- Clear/invalidate the logical mesh handle after destruction to prevent stale reuse.

### Android lifecycle

There are two distinct cases:

- **Swapchain-only recreation with the same device:** mesh buffers are device-scoped, not swapchain-scoped, so they should survive. Recreate only resources that depend on swapchain extent/images, such as framebuffers and depth targets.
- **Current full device teardown/recreation:** `closevk()` destroys the device. All GPU mesh handles become invalid. Keep CPU scene data and upload it again after the next successful device setup.

The first implementation must handle the lifecycle the code actually uses today: full device teardown on window termination.

## 6. Allocation policy

For the first bounded sea experiment, allocate the combined mesh buffer directly with `vkAllocateMemory`. It is one tiny static mesh, so a general-purpose suballocator would add complexity without a measurable benefit.

Keep the backend API independent of this choice. A suballocation arena can be considered later if measurements show that many small Vulkan allocations create meaningful overhead. Do not add VMA or another external allocator for this initial path.

## 7. Proposed backend-facing API boundary

The exact names may change during implementation, but responsibilities should remain equivalent to:

```c
/* Opaque logical resource; implementation owns VkBuffer and VkDeviceMemory. */
typedef struct TgGpuMesh TgGpuMesh;

int tg_vk_backend_upload_mesh(
    TgVkBackend *backend,
    const TgMesh3D *source,
    TgGpuMesh **out_mesh);

void tg_vk_backend_destroy_mesh(
    TgVkBackend *backend,
    TgGpuMesh *mesh);

/* Any returned VkBuffer/command buffer is borrowed for active frame recording. */
```

The scene supplies a validated `TgMesh3D`. The backend performs allocation, upload, synchronization, and destruction. The 3D renderer uses the resulting logical mesh to record indexed draws. The scene never sees `VkDeviceMemory` and never performs Vulkan allocation.

## 8. Implementation and verification sequence

1. Add a small platform-independent buffer-layout calculator with overflow and alignment tests. It must be testable by the existing host C test workflow.
2. Extract only the minimum Vulkan backend/resource functions needed for buffer creation, memory selection, staging upload, and destruction. Avoid a broad rewrite of `main.c`.
3. Add one combined static GPU mesh upload for the 20 x 20 XZ plane.
4. Add strict-warning tests for the layout calculator and run the existing math/camera/mesh suite.
5. Build the Android APK for `arm64-v8a`.
6. Only then integrate the mesh into a separate 3D graphics pipeline and depth-capable render target.
7. Run on the target Android device and capture a screenshot/video plus frame-time, FPS, memory, device, GPU/driver, and resolution details.

## 9. Explicitly out of scope for this step

- No depth attachment or 3D graphics pipeline is implemented here.
- No waves, water shader, lighting, reflections, transparency, foam, or shoreline.
- No 2D renderer rewrite or same-frame 2D/3D composition.
- No claim of runtime GPU success without an on-device run and captured evidence.
