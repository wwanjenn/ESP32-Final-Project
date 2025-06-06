<?php

if (isset($_POST["method"]) && isset($_POST["name"]) && isset($_POST["status"])) {
    $type = $_POST["method"];
    $name = $_POST["name"];
    $status = $_POST["status"];

    $servername = "localhost";
    $username = "root";
    $password = "";
    $database_name = "esp32_lockbox";

    // Create connection
    $connection = new mysqli($servername, $username, $password, $database_name);

    // Check connection
    if ($connection->connect_error) {
        die("MySQL connection failed: " . $connection->connect_error);
    }
    
    $stmt = $connection->prepare("INSERT INTO lockbox_data (type, name, status) VALUES (?, ?, ?)");
    $stmt->bind_param("sss", $type, $name, $status);

    if ($stmt->execute()) {
        echo "New record created successfully";
    } else {
        echo "Error: " . $stmt->error;
    }

    $stmt->close();
    $connection->close();
} else {
    echo "method and/or name and/or status is not set in the HTTP request";
}
?>
