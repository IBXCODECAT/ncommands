# ncommands

A personal suite of custom, lightweight command-line utilities written in C to enhance my Arch Linux workflow.

This repository houses bespoke tools designed to streamline everyday tasks, provide custom functionality, and deepen my understanding of system programming. All commands are prefixed with `n` (e.g., `ntree`, `nnote`) to prevent naming conflicts with existing system utilities and to clearly identify them as my own.

---

## Installation

To get these commands up and running on your system:

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/IBXCODECAT/ncommands.git](https://github.com/IBXCODECAT/ncommands.git)
    cd ncommands
    ```

2.  **Compile the commands:**
    Each command needs to be compiled individually. Navigate into the command's directory and use `gcc`.
    For example, to compile `ntree`:
    ```bash
    cd ntree/
    gcc ntree.c -o ntree
    cd .. # Go back to the ncommands root
    ```
    Repeat this for any other command you want to install.

3.  **Place executables in your PATH:**
    It's recommended to move the compiled executables into a directory that's part of your system's `PATH` environment variable, such as `~/bin/`. This allows you to run them from anywhere.

    ```bash
    mkdir -p ~/bin/
    mv ntree/ntree ~/bin/
    # Repeat for other commands, e.g., mv nnote/nnote ~/bin/
    ```

4.  **Update your shell's PATH (if needed):**
    If `~/bin/` is not already in your `PATH`, add it to your shell's configuration file (`~/.bashrc` for Bash, `~/.zshrc` for Zsh, etc.):

    ```bash
    echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
    source ~/.bashrc # Or source ~/.zshrc
    ```
    Then, reload your shell configuration (as shown above) or open a new terminal session.

---

## Commands

Currently available commands in `ncommands`:

<details>
<summary><h3>`ntree` - Visual File Tree Viewer</h3></summary>

A custom implementation of the `tree` command, displaying directory contents in a clear, graphical tree format. It sorts directories before files and then alphabetically, providing an intuitive overview of your file system.

#### **Features:**

* Recursively lists files and directories.
* Displays a familiar tree structure with `├──`, `└──`, and `│   ` connectors.
* Sorts entries: directories first, then files, both alphabetically.
* Handles dynamic directory sizes.
* Robust memory management.

#### **Usage:**

```bash
ntree             # Displays the tree for the current directory
ntree /path/to/dir # Displays the tree for a specific directory
