```c++
void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received. Exiting..." << std::endl;
}
// Register signal handler for graceful termination
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
```

