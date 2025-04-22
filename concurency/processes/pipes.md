

```cpp
    class Pipe {
    public:
        Pipe() {
            if (pipe(pipe_fd_.data()) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
    
        ~Pipe() {
            close(pipe_fd_[0]); // Close read end
            close(pipe_fd_[1]); // Close write end
        }
    
        // Send a `unit` through the pipe
        void send(unit u) const {
            if (write(pipe_fd_[1], &u, sizeof(unit)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    
        // Receive a `unit` from the pipe
        unit receive() const {
            unit u;
            if (read(pipe_fd_[0], &u, sizeof(unit)) == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            return u;
        }
    
        int get_read_fd() const { return pipe_fd_[0]; }
        int get_write_fd() const { return pipe_fd_[1]; }
    
    private:
        std::array<int, 2> pipe_fd_; // Use std::array for better type safety and modern C++ style
    };

```
