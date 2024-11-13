import React, { useEffect, useState } from 'react';
import useWebSocket from './hooks/useWebSocket';
import axios from 'axios';
import {
    ScatterChart,
    Scatter,
    XAxis,
    YAxis,
    CartesianGrid,
    Tooltip,
    ResponsiveContainer,
    Legend,
    LineChart,
    Line,
    ComposedChart,
    Bar,
} from 'recharts';
import './App.css'; // Import your custom CSS

const App = () => {
    const [data, setData] = useState([]);

    // Function to fetch data from the API using Axios
    const fetchData = async () => {
        try {
            const response = await axios.get('http://127.0.0.1:5000/api/data');
            setData(response.data); // Update state with fetched data
        } catch (error) {
            console.error("Error fetching data:", error);
        }
    };

    // Use WebSocket to listen for data updates
    useWebSocket('http://172.20.10.4:5000', fetchData);

    // Fetch initial data when the component mounts
    useEffect(() => {
        fetchData();
    }, []);

    // Transform data to prepare for different visualizations
    const scatterData = data.flatMap(item =>
        item.rssi_values.map((rssi) => ({
            rssi,
            userRetention: item.user_retention // Assuming this is in seconds now
        }))
    );

    // Calculate average RSSI for each document
    const averages = data.map(item => {
        const totalRSSI = item.rssi_values.reduce((sum, value) => sum + value, 0);
        return {
            averageRSSI: (totalRSSI / item.rssi_values.length).toFixed(2), // Average RSSI value
            userRetention: item.user_retention // Assuming this is in seconds now
        };
    });

    // Create heatmap data based on average RSSI
    const heatmapData = [];
    const rssiRanges = [-100, -90, -80, -70, -60, -50, -40, -30]; // Define RSSI ranges
    rssiRanges.forEach((range, index) => {
        const count = averages.filter(avg => avg.averageRSSI >= range && avg.averageRSSI < (rssiRanges[index + 1] || Infinity)).length;
        heatmapData.push({ range: `${range} - ${rssiRanges[index + 1] || 'Infinity'}`, count });
    });

    // Prepare trend analysis data using timestamps
    const trendData = data.map(item => ({
        timestamp: new Date(item.timestamp).toLocaleString(), // Format timestamp to local string
        userRetention: item.user_retention // Assuming this is in seconds now
    }));

    // Calculate average, min, and max for retention values in seconds
    const retentionValues = data.map(item => item.user_retention);
    const avgRetention = (retentionValues.reduce((a, b) => a + b, 0) / retentionValues.length).toFixed(2);
    const minRetention = Math.min(...retentionValues);
    const maxRetention = Math.max(...retentionValues);

    return (
        <div className="app-container">
            <h1>RSSI vs User Retention Pattern</h1>

            <div className="chart-container">
                <div className="chart">
                    <h2>Scatter Plot</h2>
                    <ResponsiveContainer width="100%" height={300}>
                        <ScatterChart>
                            <CartesianGrid />
                            <XAxis type="number" dataKey="rssi" name="RSSI Values" domain={[-100, 0]} />
                            <YAxis type="number" dataKey="userRetention" name="User Retention (seconds)" />
                            <Tooltip cursor={{ strokeDasharray: '3 3' }} />
                            <Legend />
                            <Scatter name="Data Points" data={scatterData} fill="#8884d8" />
                        </ScatterChart>
                    </ResponsiveContainer>
                </div>

                <div className="chart">
                    <h2>Trend Analysis of User Retention Over Time</h2>
                    <ResponsiveContainer width="100%" height={300}>
                        <LineChart data={trendData}>
                            <CartesianGrid />
                            <XAxis dataKey="timestamp" />
                            <YAxis />
                            <Tooltip />
                            <Legend />
                            <Line type="monotone" dataKey="userRetention" stroke="#82ca9d" />
                        </LineChart>
                    </ResponsiveContainer>
                </div>

                <div className="chart">
                    <h2>Average RSSI Heatmap</h2>
                    <ResponsiveContainer width="100%" height={300}>
                        <ComposedChart data={heatmapData}>
                            <CartesianGrid />
                            <XAxis dataKey="range" />
                            <YAxis />
                            <Tooltip />
                            <Bar dataKey="count" fill="#8884d8" />
                        </ComposedChart>
                    </ResponsiveContainer>
                </div>
            </div>

            <div className="statistics">
                <h2>User Retention Statistics</h2>
                <p>Average Retention: {avgRetention} seconds</p>
                <p>Minimum Retention: {minRetention} seconds</p>
                <p>Maximum Retention: {maxRetention} seconds</p>
            </div>
        </div>
    );
};

export default App;
