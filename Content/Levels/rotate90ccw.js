const fs = require('fs')

function rotate90ccw(lines) {
	const newLines = []
	for (let y = lines[0].length - 1; y >= 0; y--) {
		let line = ''
		for (let x = 0; x < lines.length; x++) {
			line += lines[x][y]
		}
		newLines.push(line)
	}
	return newLines
}

function transformFile(fromFile, toFile) {
	const fileContent = fs.readFileSync(fromFile, 'utf8')
	const lines = fileContent.split('\n')
	const rotatedLines = rotate90ccw(lines)
	fs.writeFileSync(toFile, rotatedLines.join('\n'))
}

transformFile('test_level.txt', 'test_level_rotated.txt')
