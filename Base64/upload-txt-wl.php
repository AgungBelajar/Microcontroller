<?php
$servername = "localhost";
$dbname = "u104419741_iot";
$username = "u104419741_iot";
$password = "Srsssm5cd@";

$conn = mysqli_connect($servername, $username, $password, $dbname);
if (mysqli_connect_errno()) {
    die("Failed to connect to MySQL: " . mysqli_connect_error());
} else {
    if ($_SERVER['REQUEST_METHOD'] == 'POST') {
        $fileData = isset($_POST['fileData']) ? $_POST['fileData'] : '';
    
        if ($fileData) {
            $decodedData = base64_decode($fileData);
            $lines = preg_split('/\r\n|\r|\n/', $decodedData);
            
            $successfulInserts = 0;
            $totalLines = 0;
            foreach ($lines as $line) {
                $line = trim($line);
                if (!empty($line)) {
                    $totalLines++;
                    parse_str($line, $params);
    
                    $lvl_in = isset($params['lvl_in']) ? $params['lvl_in'] : 0;
                    $date = isset($params['d']) ? $params['d'] : '';
                    $idwl = isset($params['idwl']) ? $params['idwl'] : 0;
                    
                    if ($idwl != 0 && !empty($date)) {
                        $sqlCheck = "SELECT * FROM water_level WHERE idwl = '$idwl' AND datetime = '$date'";
                        $resultCheck = mysqli_query($conn, $sqlCheck);
                        
                        if (mysqli_num_rows($resultCheck) <= 0) {
                            $sqlInsert = "INSERT INTO water_level (idwl, datetime, lvl_in) VALUES ('$idwl', '$date', '$lvl_in')";
                            $resultInsert = mysqli_query($conn, $sqlInsert);
                            
                            if ($resultInsert) {
                                $successfulInserts++;
                            }
                        }
                    }
                }
            }
            
            if ($successfulInserts == 0) {
                die("Tidak ada data yang ditambahkan ke database.");
            } elseif ($successfulInserts == $totalLines) {
                die("Semua data berhasil ditambahkan ke database.");
            } else {
                die("Beberapa data berhasil ditambahkan ke database.");
            }
        } else {
            die("No file data received.");
        }
    } else {
        die("Invalid request method.");
    }
}
?>
