import('node-fetch').then(mod => {
	const fetch = mod.default;

	if (process.argv.length < 3) {
		console.log('Usage: node scripts/make-gitlab-release <changelog>');
		process.exit(1);
	}

	const version = process.env['CI_COMMIT_TAG'];
	const changelog = process.argv[2];

	console.log(`Creating GitLab release for version: ${version}`);

	fetch(
		`https://gitlab.paymoapp.com/api/v4/projects/${process.env['CI_PROJECT_ID']}/releases`,
		{
			method: 'POST',
			headers: {
				'JOB-TOKEN': process.env['CI_JOB_TOKEN']
			},
			body: JSON.stringify({
				name: version,
				tag_name: version,
				description: changelog
			})
		}
	);
});
