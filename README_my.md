1. Need to make a function to receive the registration details of the storage servers ( done )

2. So my first step would be to initialize all storage servers then i need to accept client requests 

3. At any time i not only need to handle client requests but i also need to handle cases when a new storage server needs to be initialzed . 

4. I can issue commands to SS which are 
    1. Create an Empty File/Directory: The Naming Server can instruct a Storage Server to create an empty file or directory, initiating the storage of new data.
    2. Delete a File/Directory: Storage Servers can receive commands to delete files or directories, thereby freeing up storage space and maintaining data consistency.
    3. Copy Files/Directories: Storage Servers can copy files or directories from other Storage Servers, with the NM providing the relevant IP addresses for efficient data transfer.

5. While interacting with clients i will take his request and all provide the details of the respective SS and i need to return the IP and port of the SS to the client 
        1. Path finding: Say, the client passes READ dir1/dir2/file.txt to the NM -> the NM looks over all the global paths in SS1, SS2, SS3… then sees that the path is present in SSx -> The NM gives relevant information about SSx to the client.
        2. Client Request: Clients can initiate file-related operations such as reading, writing, obtaining information or streaming audio files by providing the file’s path. The NM, upon receiving the request, takes on the responsibility of locating the correct SS where the file is stored.
        NM Facilitation: The NM identifies the correct Storage Server and returns the precise IP address and client port for that SS to the client.

        EG. 
            Client : READ <path>
            NS looks for the given path in list of accessible paths. If found return IP and Port.
            NS (to client) : IP: 10.2.141.242 Port: 5050


6.  
    1. Clients can request file and folder creation or deletion operations by providing the respective path and action (create or delete). Once the NM determines the correct SS, it forwards the request to the appropriate SS for execution.
    2. SS Execution: The SS processes the request and performs the specified action, such as creating an empty file or deleting a file/folder.
    3. Acknowledgment and Feedback: After successful execution, the SS sends an acknowledgment (ACK) or a STOP packet to the NM to confirm task completion. The NM, in turn, conveys this information back to the client, providing feedback on the task’s status.

    EG. 
        Client : CREATE <path> <name>
        NS looks whether the given path is available for creation.
        NS (to SS) : Sends the path and name to be created
        SS sends acknowledgement to NM.
        NM sends acknowledgment to Client.
        Note : NM will also dynamically add the new path in this case into the list of accessible paths.

7. 
    1.  Clients can request to copy files or directories between SS by providing both the source and destination paths.The NM, upon receiving this request  
    2. NM Execution: The NM orchestrates the copying process, ensuring that data is transferred accurately. This operation may involve copying data from one SS to another, even if both source and destination paths lie within the same SS.
    3. Acknowledgment and Client Notification: Upon the successful completion of the copy operation, the NM sends an acknowledgment (ACK) packet to the client. This ACK serves as confirmation that the requested data transfer has been successfully executed.

    EG. 
        Client : COPY <source> <dest>
        NS looks whether the given source and destination paths is available .(Handle all the different types of source and destination paths possible)
        Handle how you want to do copy operation between different SSs or among same SS
        Destination SS sends acknowledgement to NM.
        NM sends acknowledgment to Client.

8. 
    1. Listing all accessible paths:
    Client Request: Client requests the NS to inform about all accessible paths available. This helps the client to get an idea of what files it can access.
    2. NM Execution : The NS provides all the accessible paths across all registered storage servers to the client.



9. For searching algo :

    Efficient Search: Optimize the search process employed by the Naming Server when serving client requests. Avoid linear searches and explore more efficient data structures such as Tries and Hashmaps to swiftly identify the correct Storage Server (SS) for a given request. This optimization enhances response times, especially in systems with a large number of files and folders.

10. Do proper error handling for eah part : 

    Error Handling: Define a set of error codes that can be returned when a client’s request cannot be accommodated. For example, the NM should return distinct error codes for situations where a file is unavailable because it doesn’t exist and when another client is currently writing to the file. Clear and descriptive error codes enhance the communication of issues between the NFS and clients

11. Bookkeeping :   

    Logging and Message Display: Implement a logging mechanism where the NM records every request or acknowledgment received from clients or Storage Servers. Additionally, the NM should display or print relevant messages indicating the status and outcome of each operation. This bookkeeping ensures traceability and aids in debugging and system monitoring.
    IP Address and Port Recording: The log should include relevant information such as IP addresses and ports used in each communication, enhancing the ability to trace and diagnose issues.


12. Later part :
    1. Asynchronous and Synchronous writing
    2. Concurrent File Reading 
    3. LRU Caching
    4. Backing up Data
    5. Redundancy 


Also, even when there are no storage servers, your client can still send requests but it doesn't make sense at that point since there are no accessible paths.

However a storage server can register at any time during the execution of the program.