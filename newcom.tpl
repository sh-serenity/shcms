<script type=\"text/javascript\">$(document).ready(function() { $(\"#test1\").on('submit', (function(e) {e.preventDefault();
$.ajax({url: \"addmsg\", type: "post", data: {test1: $('#test1').html();}, success: function(data) {$(\"#test1\").html(data);}
});return false;}));});</script>
