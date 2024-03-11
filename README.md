# Reports Management System

This application is designed to manage the operations of a manufacturing company. It includes a daemon that continually manages the operations, logging of new or modified xml reports, backing up of the dashboard directory content, moving of uploaded reports to the reporting directory, and inter-process communication (IPC).

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

You will need a Linux environment with GCC compiler installed.

### Installing

1. Clone the repository to your local machine.
2. Navigate to the project directory.
3. Run `make` command to compile the project and create executables.

## Running the Application

1. Start the daemon by running the executable created in the `src/daemon` directory.
2. Department managers can upload their xml report files to the respective directories in `reports`.
3. The daemon will automatically move the uploaded reports to the reporting directory at 1am.
4. The dashboard directory content is backed up every night.
5. Details of new or modified xml reports are logged and can be viewed in the `logs` directory.

## Built With

* C - The programming language used

## Authors

* Matthew Tweedy

## License

This project is licensed under the MIT License.
