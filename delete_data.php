<?php
$servername = "localhost";
$username = "root";
$password = "";
$database_name = "esp32_lockbox";

if (!isset($_GET['id'])) {
    die("No ID provided.");
}

$entry_id = intval($_GET['id']);

$conn = new mysqli($servername, $username, $password, $database_name);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Securely delete using prepared statement
$stmt = $conn->prepare("DELETE FROM lockbox_data WHERE id = ?");
if ($stmt === false) {
    die("Prepare failed: " . $conn->error);
}

$stmt->bind_param("i", $entry_id); // 'i' means integer

if ($stmt->execute()) {
    echo "Entry deleted successfully.";
} else {
    echo "Error deleting entry: " . $stmt->error;
}

$stmt->close();
$conn->close();
?>
