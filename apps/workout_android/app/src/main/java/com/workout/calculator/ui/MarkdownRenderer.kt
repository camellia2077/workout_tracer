package com.workout.calculator.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp

@Composable
fun MarkdownContent(
    markdown: String,
    modifier: Modifier = Modifier,
) {
    val blocks = parseMarkdownBlocks(markdown)
    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        blocks.forEach { block ->
            when (block) {
                is MarkdownBlock.Heading -> {
                    val textStyle = when (block.level) {
                        1 -> MaterialTheme.typography.headlineSmall
                        2 -> MaterialTheme.typography.titleLarge
                        else -> MaterialTheme.typography.titleMedium
                    }
                    Text(
                        text = block.text,
                        style = textStyle,
                    )
                }

                is MarkdownBlock.Bullet -> {
                    Row {
                        Text(
                            text = "• ",
                            style = MaterialTheme.typography.bodyMedium,
                        )
                        Text(
                            text = toInlineMarkdownText(block.text),
                            style = MaterialTheme.typography.bodyMedium,
                        )
                    }
                }

                is MarkdownBlock.Paragraph -> {
                    Text(
                        text = toInlineMarkdownText(block.text),
                        style = MaterialTheme.typography.bodyMedium,
                    )
                }

                is MarkdownBlock.CodeBlock -> {
                    Text(
                        text = block.code,
                        style = MaterialTheme.typography.bodySmall.copy(fontFamily = FontFamily.Monospace),
                        modifier = Modifier
                            .fillMaxWidth()
                            .background(Color(0x22000000))
                            .padding(10.dp),
                    )
                }
            }
        }
    }
}

private sealed interface MarkdownBlock {
    data class Heading(val level: Int, val text: String) : MarkdownBlock
    data class Bullet(val text: String) : MarkdownBlock
    data class Paragraph(val text: String) : MarkdownBlock
    data class CodeBlock(val code: String) : MarkdownBlock
}

private fun parseMarkdownBlocks(markdown: String): List<MarkdownBlock> {
    val result = mutableListOf<MarkdownBlock>()
    val codeBuffer = mutableListOf<String>()
    var inCodeBlock = false

    markdown.lines().forEach { rawLine ->
        val line = rawLine.trimEnd()
        val trimmed = line.trim()

        if (trimmed.startsWith("```")) {
            if (inCodeBlock) {
                result += MarkdownBlock.CodeBlock(codeBuffer.joinToString("\n"))
                codeBuffer.clear()
            }
            inCodeBlock = !inCodeBlock
            return@forEach
        }

        if (inCodeBlock) {
            codeBuffer += line
            return@forEach
        }

        when {
            trimmed.isEmpty() -> Unit
            trimmed.startsWith("# ") -> result += MarkdownBlock.Heading(1, trimmed.removePrefix("# ").trim())
            trimmed.startsWith("## ") -> result += MarkdownBlock.Heading(2, trimmed.removePrefix("## ").trim())
            trimmed.startsWith("### ") -> result += MarkdownBlock.Heading(3, trimmed.removePrefix("### ").trim())
            trimmed.startsWith("- ") -> result += MarkdownBlock.Bullet(trimmed.removePrefix("- ").trim())
            else -> result += MarkdownBlock.Paragraph(trimmed)
        }
    }

    if (codeBuffer.isNotEmpty()) {
        result += MarkdownBlock.CodeBlock(codeBuffer.joinToString("\n"))
    }
    return result
}

private fun toInlineMarkdownText(text: String): AnnotatedString {
    return buildAnnotatedString {
        var index = 0
        while (index < text.length) {
            if (text.startsWith("**", index)) {
                val end = text.indexOf("**", index + 2)
                if (end > index + 1) {
                    val content = text.substring(index + 2, end)
                    pushStyle(SpanStyle(fontWeight = FontWeight.Bold))
                    append(content)
                    pop()
                    index = end + 2
                    continue
                }
            }

            if (text[index] == '`') {
                val end = text.indexOf('`', index + 1)
                if (end > index) {
                    val content = text.substring(index + 1, end)
                    pushStyle(
                        SpanStyle(
                            fontFamily = FontFamily.Monospace,
                            background = Color(0x22000000),
                        )
                    )
                    append(content)
                    pop()
                    index = end + 1
                    continue
                }
            }

            append(text[index])
            index += 1
        }
    }
}
