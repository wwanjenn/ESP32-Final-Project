<?php
$servername = "localhost";
$username = "root";
$password = "";
$database_name = "esp32_lockbox";

// Create connection
$conn = new mysqli($servername, $username, $password, $database_name);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

$sql = "SELECT id, type, name, timestamp, status FROM lockbox_data ORDER BY timestamp DESC";

$result = $conn->query($sql);

$timestamps = [];
$names = [];
$types = [];
$statuses = [];
$rows = [];

if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $timestamps[] = $row['timestamp'];
        $names[] = $row['name'];
        $types[] = $row['type'];
        $statuses[] = $row['status'];
        $rows[] = $row;
    }
} else {
    echo "No data found.";
    exit();
}

$conn->close();
?>
