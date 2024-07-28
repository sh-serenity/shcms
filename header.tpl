
<!doctype html>
<html lang="ru-RU">
<head>
<meta charset="utf-8">
<title>dev.shushik.kiev.ua</title>
    <meta http-equiv="Content-Type" content="text/html"; charset=utf-8">
    <link rel="stylesheet" href="/style.css">
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script>
    <script src="tinymce/js/tinymce/tinymce.min.js"></script>
    <script>
    tinymce.init({
    selector: 'textarea',
    force_br_newlines : true,
    force_p_newlines : false,
    menubar: false, statusbar: false, toolbar: false, height: 100, width: 550,branding: false
    });
    </script>  

    <script>
    function reloadPage() {
var currentDocumentTimestamp =
new Date(performance.timing.domLoading).getTime();
var now = Date.now();
var tenSec = 10 * 1000;
var plusTenSec = currentDocumentTimestamp + tenSec;
if (now > plusTenSec) {
location.reload();
} else {}
}
</script>
</head>
<body>
<div class=logo><h1>Thanks to Brian W. Kernighan, Dennis M. Ritchie.....</h1></div>
<div class="sh"><center><img src="pdn.png"></center></div>