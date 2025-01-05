# Multi-Core Producer-Consumer with Character Device Queue

This project implements a **multi-core producer-consumer** system that interacts with a custom **Linux character device** (`/dev/myQueue`) using semaphores, mutexes, and thread affinity for optimized performance on both single-core and multi-core setups.

The project consists of a **user-space application** and a **kernel module** for managing the queue.

---

## Features
- **Producer-Consumer Threads**:
  - A writer (producer) writes data to the queue.
  - Multiple readers (consumers) read data concurrently.
- **Single-Core and Multi-Core Modes**:
  - Threads are pinned to specific CPU cores for performance comparison.
- **Linux Character Device** (`/dev/myQueue`):
  - Acts as the shared queue, implemented in the kernel module.
- **Synchronization**:
  - Mutexes and semaphores ensure safe access to shared resources.

---

## File Structure
### 1. **Kernel Module**
- **`queue_module.c`**: Implements the queue logic for `/dev/myQueue`.
- **Makefile**:
  - Builds the kernel module.
  - Provides commands for installing, uninstalling, and cleaning the module.

### 2. **User-Space Application**
- **`main.c`**: Implements the producer-consumer logic with thread management, core pinning, and performance measurement.
- **Makefile**:
  - Builds and runs the user-space application (`user`).
  - Handles queue permissions and module installation/uninstallation.

---

## Prerequisites
- A Linux environment with kernel headers installed.
- **GCC** for compiling the user-space program.
- Root privileges for loading/unloading kernel modules.

---

## How to Build and Run
### 1. **Kernel Module**
```bash
make        # Build the kernel module
make install # Install the kernel module
make permission # Set device permissions
```

### 2. **User-Space Application**
```bash
make user   # Compile and run the user-space application
```

### 3. **Clean Up**
```bash
make clean      # Clean build files
make uninstall  # Unload the kernel module
```

---

## Testing Modes
1. **Single-Core Mode**:
   - All threads are pinned to **Core 0**.
   - Useful for analyzing performance under constrained CPU conditions.

2. **Multi-Core Mode**:
   - Threads are distributed across available CPU cores.
   - Exploits parallelism for performance gains.

---

## Example Output
- **Single-Core Mode**:
  - Time taken for producer and consumer operations on a single core.
- **Multi-Core Mode**:
  - Time taken for distributed operations across multiple cores.

---

## Advanced Notes
- Modify `BLOCKING` in the Makefile to toggle blocking behavior in the queue module:
  ```bash
  make BLOCKING=0 install
  ```
- The `writer` and `reader` threads synchronize via semaphores (`sem_empty` and `sem_full`) and access the queue device through file descriptors.

---

## License
This project is released under the **MIT License**.

For additional details or contributions, feel free to create an issue or submit a pull request.

