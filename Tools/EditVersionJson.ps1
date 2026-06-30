$filePath = $args[0]
$version = $args[1]
$jsonField = $args[2]

if (!$jsonField) { $jsonField = "EngineVersion" }

$config = [System.IO.File]::ReadAllLines($filePath) | ConvertFrom-Json
$config.$jsonField = "$version"
$config | ConvertTo-Json -Depth 10 | Out-File $filePath -Encoding utf8
