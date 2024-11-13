// src/hooks/useWebSocket.js
import { useEffect, useState } from 'react';
import { io } from 'socket.io-client';

const useWebSocket = (url, fetchData) => {
    useEffect(() => {
        const socket = io(url);

        socket.on('data_updated', () => {
            fetchData(); // Fetch new data when notified
        });

        return () => {
            socket.disconnect();
        };
    }, [url, fetchData]);
};

export default useWebSocket;
