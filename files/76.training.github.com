<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
  <title>GitHub Training</title>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <link rel="stylesheet" href="/stylesheets/page.css">


  <link rel="icon" type="image/x-icon" href="/favicon.png" />
  <link rel="alternate" type="application/atom+xml" title="Upcoming Events" href="/atom.xml">
  <script type="text/javascript" src="/js/jquery.min.js"></script>
  <script type="text/javascript" src="/js/quotes.js"></script>
  <script type="text/javascript" src="/js/all.js"></script>

  <!-- Latest compiled and minified CSS -->
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap.min.css">

  <!-- Optional theme -->
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap-theme.min.css">

  <!-- Latest compiled and minified JavaScript -->
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/js/bootstrap.min.js"></script>

  <!-- google analytics async -->
<script type="text/javascript">
  var _gaq = _gaq || [];
  _gaq.push(['_setAccount', 'UA-3769691-19']);
  _gaq.push(['_setDomainName', 'github.com']);
  _gaq.push(['_trackPageview']);

  (function() {
    var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
    ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
    var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
  })();
</script>

</head>

<body class="home">
  <header class="colorful">

	<div class="container">
		<div class="row">
			<div class="col-md-3 col-sm-2">
				<button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#main-navbar-collapse">
					<span class="sr-only">Toggle navigation</span>
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
					<span class="icon-bar"></span>
				</button>
				<div id="logo"><a href="/" data-escp="">GitHub Training</a></div>
			</div>

			<div class="col-md-9 col-sm-10">
				<div class="collapse navbar-collapse navbar-right" id="main-navbar-collapse">
					<ul class="nav navbar-nav navbar-right">
						<li><a href="/trainers/">Trainers</a></li>
						<li><a href="/classes/">Classes</a></li>
						<li><a href="/schedule/">Schedule</a></li>
						<li><a href="/kit/">Kit</a></li>
						<li><a href="/contact/">Contact</a></li>
					</ul>
				</div>
			</div>
		</div>

		


		
			<div class="jumbotron colorful">
				<div class="container">
					<div class="row">
						<div class="col-md-6">
							<img src="/images/git-training-pics.png" alt="GitHub Training at Live Event" class="img-responsive">
						</div>
						<div class="col-md-6">
							<h2>Education from the experts</h2>
							<p>We love helping people just like you use GitHub and Git to collaborate  and be more productive. No matter your level of experience, we’ve got something just for you.</p>
						</div>
					</div>
				</div>
			</div>
		
	</div>
</header>


  


  <div class="container">
  <div class="col-md-4">
    <h3>Online Training</h3>
    <p>No matter where you are in the world, we have live web-based classes that you can access from the convenience of your own computer.</p>
    <p class="more"><a href="/classes/" class="">Learn from anywhere »</a></p>
  </div>

  <div class="col-md-4">
    <h3>In-person Training</h3>
    <p>For face-to-face training, our highly experienced trainers can help increase your team's profiency with a live, in-person class.</p>
    <p class="more"><a href="/contact/" class="">Schedule a session »</a></p>
  </div>

  <div class="col-md-4">
    <h3>Training Resources</h3>
    <p>We've open sourced all of our class outlines, slides, and cheat sheets so you can easily leverage these high-quality educational materials.</p>
    <p class="more"><a href="/kit/" class="">Browse our resources »</a></p>
  </div>
</div>

<div class="container well text-center">
  <h4>
    Got 15 minutes and want to learn the basics of Git?
    <a href="http://try.github.com">
      <img src="/images/try-git-logo.png" alt="Try Git in your browser for free" width="30px" height="30px">
      Try Git right in your web browser.
    </a>
  </h4>
</div>

<div class="container">
  <div class="row">
    <div class="col-md-6">
      
        <h4>Get in touch, today!</h4>
        <p>We love hearing from people, so if you have any questions about GitHub’s training products and services, get in touch right away.</p>
        <p class="more"><a href="/contact/" class="">Contact our training team »</a></p>
      
    </div>

    <div class="col-md-6">
      <h4>What people are saying about GitHub Training...</h4>

      <div class="testimonial">
        <blockquote>
          <p id="js-quote-text"></p>
        </blockquote>
        <div class="attribution"><span id="js-quote-author"></span>, <span id="js-quote-company"></span></div>
      </div>

    </div>
  </div>
</div>



  <footer>
  <div class="container">
    <div class="row">
      <div class="col-md-5">
        <ul class="text-left">
          <li>&copy; 2015 <span>GitHub</span>, Inc.</li>
          <li><a href="https://github.com/site/terms">Terms</a></li>
          <li><a href="https://training.github.com/kit/license.html">License</a></li>
          <li><a href="https://github.com/security">Security</a></li>
          <li><a href="https://training.github.com/contact">Contact</a></li>
        </ul>
      </div>

      <div class="col-md-2">
        <a href="/"><span class="mega-octicon octicon-mark-github"></span></a>
      </div>

      <div class="col-md-5">
        <ul class="text-right">
          <li><a href="https://twitter.com/GitHubTraining"><span class="octicon octicon-mark-twitter"></span>githubtraining</a></li>
          <li><a href="https://status.github.com">Status</a></li>
          <li><a href="https://github.com/blog">Blog</a></li>
          <li><a href="https://github.com/about">About</a></li>
        </ul>
      </div>
    </div>
  </div>
</footer>

</body>
</html>

</html>
