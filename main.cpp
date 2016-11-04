#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unordered_map>

struct prevkeys
{
	int a, b, c, d;
	prevkeys()
	{
		reset();
	}
	void reset()
	{
		a = b = c = d = -1;
	}
	void shift()
	{
		d = c;
		c = b;
		b = a;
		a = -1;
	}
	bool operator==(const prevkeys &other) const
	{
		return (a == other.a) && (b == other.b) && (c == other.c) && (d == other.d);
	}
};

namespace std {

	template <>
	struct hash<prevkeys>
	{
		std::size_t operator()(const prevkeys& k) const
		{
			return (k.a << 1)^(k.b << 3)^(k.c << 2)^(k.d);
		}
	};

}

int whitespace(char aCharacter)
{
	switch (aCharacter)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return 1;
	}
	return 0;
}

int is_numeric(char *aString)
{
	if (!*aString) return 0;
	while (*aString)
	{
		if (*aString < '0' || *aString > '9')
			return 0;
		aString++;
	}
	return 1;
}

int is_alpha(char *aChar)
{
	if (!*aChar) return 0;
	if (toupper(*aChar) < 'A' || toupper(*aChar) > 'Z')
		return 0;
	return 1;
}


class WordCounter
{
public:
	std::unordered_map<int, char*> mWord;
	std::unordered_map<int, int> mFlags, mHits, mHash;
	std::unordered_map<prevkeys, std::unordered_map<int, int>> mLongHit;
	prevkeys mPrevKeys;
	int mPrevToken;
	int mTokens;
    int mWordno;	

	int calcHash(char *aString)
	{
		unsigned int i = 0;
		while (*aString)
		{
			i = (i << 11) | (i >> 21);
			i ^= *aString;
	        aString++;
		}    
    
		return i;
	}    

	WordCounter()
	{
		mPrevToken = -1;
		mTokens = 0;
		mWordno = 0;		
	}

	void tokenRef(int aCurrent)
	{
		mLongHit[mPrevKeys][aCurrent]++;

		// Populate shorter hits as well
		if (mPrevKeys.d != -1)
		{
			prevkeys t = mPrevKeys;
			t.d = -1;
			mLongHit[t][aCurrent]++;
			if (mPrevKeys.c != -1)
			{
				t.c = -1;
				mLongHit[t][aCurrent]++;
				if (mPrevKeys.b != -1)
				{
					t.b = -1;
					mLongHit[t][aCurrent]++;
				}
			}
		}

		mPrevKeys.shift();
		mPrevKeys.a = aCurrent;

		if (mWordno == 0)
			mPrevKeys.reset();
	}

	void addWordcountToken(char *aToken)
	{
		int h = calcHash(aToken);
		int i;
		if (aToken == NULL || *aToken == 0)
			return;

		for (i = 0; i < mTokens; i++)
		{
			if (mHash[i] == h && strcmp(mWord[i], aToken) == 0)
			{
				tokenRef(i);
				mHits[i]++;
				mFlags[i] |= (mWordno == 0) ? 2 : (mWordno == 1 && toupper(*aToken) == *aToken) ? 1 : 0;
				return;
			}
		}

		mWord[mTokens] = _strdup(aToken);
		mHash[mTokens] = h;
		mHits[mTokens] = 1;
		mFlags[mTokens] = (mWordno == 0) ? 2 : (mWordno == 1 && toupper(*aToken) == *aToken) ? 1 : 0;
		tokenRef(mTokens);
		mTokens++;
	}

	int valid(char *aToken)
	{
		int allupper = 1;
		int len = 0;
		while (*aToken)
		{
			if ((*aToken >= 'a' && *aToken <= 'z') ||
				(*aToken >= 'A' && *aToken <= 'Z') ||
				*aToken == '\'' ||
				*aToken == ':' ||
				*aToken == ';' ||
				*aToken == '.' ||
				*aToken == '?' ||
				*aToken == '!')
			{
				// Probably valid.
				if ((*aToken == ':' ||
					*aToken == ';' ||
					*aToken == '.' ||
					*aToken == '?' ||
					*aToken == '!'))
				{
					if (*(aToken + 1) != 0)
					{
						return 0; // not valid if not at the end of a string.
					}
					else
					{
						if (mWordno == 0)
						{
							return 0; // and not valid if the only word in the sentence
						}
						if (allupper && len > 1)
						{
							mWordno = 0; // all uppercase so we'll skip, but consider end of sentence anyway.
							return 0;
						}
						if (len < 2 && !is_alpha(aToken - 1))
						{
							return 0;
						}

						mWordno = 0; // otherwise valid, and ends the sentence
                        return 1;
                    }                
                }
                if (*aToken >= 'a' && *aToken <= 'z')
                    allupper = 0;
	        }
	        else
	        {
	            return 0;
	        }	                
	        aToken++;
	        len++;
	    }
	    if (allupper && len > 1)
	        return 0;
		if (len < 2 && !is_alpha(aToken - 1))
			return 0;
		mWordno++;
	    return 1;
	}
	
	void wordCount(char *aString)
	{
		char temp[256];
		int p = 0;
		while (*aString)
		{
			temp[p] = *aString;
			if (*aString == '_' || 
			    *aString == '-' || 
			    *aString == '"' || 
			    *aString == ' ' || 
			    *aString == 0 || 
			    *aString == '\t' || 
			    *aString == '\r' || 
			    *aString == '\n')
			{
				temp[p] = 0;
				if (p > 0)
				{
					if (valid(temp))
						addWordcountToken(temp);
					else
						mPrevKeys.reset();
				}
				p = 0;
			}
			else
			{
				p++;
			}
			aString++;
		}
		temp[p] = 0;
		if (valid(temp))
		addWordcountToken(temp);
	}
};

WordCounter gWordCounter;




void read_raw_line(char *aBuffer, FILE * aFile)
{
    int i = 0, count = 0;
    int c;
    do
    {
        c = fgetc(aFile);
        if (c == '\r')
            c = fgetc(aFile);
        if (c == '\t')
            c = ' ';
        if (feof(aFile))
            c = '\n';
        aBuffer[i] = c;
        if (!feof(aFile) && c != '\n')
            i++;
		count++;
    }
    while (!feof(aFile) && c > 31 && !(count > 1000 && c == 32));
    
    // trim trailing spaces:
    while (i >= 0 && whitespace(aBuffer[i])) i--;
    i++;
    
    // terminate
    aBuffer[i] = 0;
}

// Skips empty and commented lines
void read_line(char *aBuffer, FILE * aFile)
{
    do
    {
        read_raw_line(aBuffer, aFile);
    }
    while (!feof(aFile) && aBuffer[0] == '#' && aBuffer[0] > 31);
}

void token(int aTokenIndex, char *aSource, char *aDestination)
{    
    while (aTokenIndex && *aSource)
    {
        while (*aSource && !whitespace(*aSource)) aSource++;
        while (*aSource && whitespace(*aSource)) aSource++;
        aTokenIndex--;
    }
    while (*aSource && !whitespace(*aSource))
    {
        *aDestination = *aSource;
        aDestination++;
        aSource++;
    }
    *aDestination = 0;
}

int tokencmp_for_qsort(const void * a, const void * b)
{
	int idx1 = *(int*)a;
	int idx2 = *(int*)b;
	return gWordCounter.mHits[idx2] - gWordCounter.mHits[idx1];
}

int findflag(int val, int flag)
{
    int i;
    int last = 0;
    for (i = 0; i < gWordCounter.mTokens; i++)
    {
        if ((gWordCounter.mFlags[i] & flag) != 0)
        {
            last = i;
            val -= gWordCounter.mHits[i];
            if (val <= 0)
                return i;
        }
    }
    return last;
}

int findstarter(int val)
{
	return findflag(val, 1);
}

int findender(int val)
{
	return findflag(val, 2);
}

int countflag(int flag)
{
	int i, ssum;
	ssum = 0;
	for (i = 0; i < gWordCounter.mTokens; i++)
	{
		if ((gWordCounter.mFlags[i] & 1) != 0) { ssum += gWordCounter.mHits[i]; }
	}
	return ssum;
}

int countinstflag(int flag)
{
	int i, ssum;
	ssum = 0;
	for (i = 0; i < gWordCounter.mTokens; i++)
	{
		if ((gWordCounter.mFlags[i] & 1) != 0) { ssum++; }
	}
	return ssum;
}

prevkeys gPrevKeys;
int gWordNumber;

//#define DEBUGPRINT

int nextword(int w)
{
	int sum = 0, n = 0;
	gPrevKeys.shift();
	gPrevKeys.a = w;
	prevkeys t = gPrevKeys;
	for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
	{
		n++;
		sum += it->second * 80; 
		if (gWordCounter.mFlags[it->second] & 2) sum += gWordNumber * gWordNumber / 10;
	}

	if (t.d != -1)
	{
		t.d = -1;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			n++;
			sum += it->second * 40;
			if (gWordCounter.mFlags[it->second] & 2) sum += gWordNumber * gWordNumber / 10;
		}
	}
	if (t.c != -1)
	{
		t.c = -1;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			n++;
			sum += it->second * 20;
			if (gWordCounter.mFlags[it->second] & 2) sum += gWordNumber * gWordNumber / 10;
		}
	}
	if (t.b != -1)
	{
		t.b = -1;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			n++;
			sum += it->second;
			if (gWordCounter.mFlags[it->second] & 2) sum += gWordNumber * gWordNumber / 10;
		}
	}

#ifdef DEBUGPRINT
	printf("[%c%dz%d", 
		(gPrevKeys.d != -1) ? 'd' : (gPrevKeys.c != -1) ? 'c' : (gPrevKeys.b != -1) ? 'b' : (gPrevKeys.a != -1) ? 'a' : '?',
		n, sum);
#endif
	if (sum == 0)
	{
#ifdef DEBUGPRINT
		printf("]");
#endif
		return findender(rand() % countflag(2));
	}
	sum = rand() % sum;
	int latest;
	t = gPrevKeys;
	for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
	{
		sum -= it->second * 80;
		if (gWordCounter.mFlags[it->second] & 2) sum -= gWordNumber * gWordNumber / 20;
		if (sum <= 0)
		{
#ifdef DEBUGPRINT
			printf("%c]", (gPrevKeys.d != -1) ? 'd' : (gPrevKeys.c != -1) ? 'c' : (gPrevKeys.b != -1) ? 'b' : (gPrevKeys.a != -1) ? 'a' : '?');
#endif
			return it->first;
		}
		latest = it->first;
	}
	if (t.d != -1)
	{
		t.d = -1;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			sum -= it->second * 40;
			if (gWordCounter.mFlags[it->second] & 2) sum -= gWordNumber * gWordNumber / 20;
			if (sum <= 0)
			{
#ifdef DEBUGPRINT
				printf("c]");
#endif
				return it->first;
			}
			latest = it->first;
		}
	}
	if (t.c != -1)
	{
		t.c = -1;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			sum -= it->second * 20;
			if (gWordCounter.mFlags[it->second] & 2) sum -= gWordNumber * gWordNumber / 20;
			if (sum <= 0)
			{
#ifdef DEBUGPRINT
				printf("b]");
#endif
				return it->first;
			}
			latest = it->first;
		}
	}
	if (t.b != -1)
	{
		t.b = -1;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			sum -= it->second;
			if (gWordCounter.mFlags[it->second] & 2) sum -= gWordNumber * gWordNumber / 20;
			if (sum <= 0)
			{
#ifdef DEBUGPRINT
				printf("a]");
#endif
				return it->first;
			}
			latest = it->first;
		}
	}
#ifdef DEBUGPRINT
	printf("?]");
#endif
	return latest;
}

int main(int parc, char**pars)
{
    if (parc < 2)
    {
        printf("Gimme a txt file\n");
        return 0;
    }
    
    FILE * f = fopen(pars[1], "r");
    char scratch[8192];
    
	if (!f)
	{
		printf("File not found\n");
		return 0;
	}

	int lines = 0;

    while (!feof(f))// && lines < 512)
    {
		lines++;
		if ((lines & 511) == 0)
		printf("%d lines, %d tokens\r", lines, gWordCounter.mTokens);
        read_line(scratch, f);
        gWordCounter.wordCount(scratch);
    }
        
    fclose(f);
    
    int *idx = new int[gWordCounter.mTokens];
    int i;
	for (i = 0; i < gWordCounter.mTokens; i++)
        idx[i] = i;
        
	qsort(idx, gWordCounter.mTokens, sizeof(int), tokencmp_for_qsort);
	printf("\n\n");
	printf("%d tokens total, %d starters, %d enders\n", gWordCounter.mTokens, countinstflag(1), countinstflag(2));
	printf("Most frequent words in source material:\n");
	int total = gWordCounter.mTokens;
	if (total > 99) total = 99;
	total /= 3;
    for (i = 0; i < total; i++)
    {
        printf("%2d. %-10s(%5d) %2d. %-10s(%5d) %2d. %-10s(%5d)\n",
			i+1, gWordCounter.mWord[idx[i]], gWordCounter.mHits[idx[i]],
			i+1+total, gWordCounter.mWord[idx[i+total]], gWordCounter.mHits[idx[i+total]],
			i+1+total*2, gWordCounter.mWord[idx[i+total*2]], gWordCounter.mHits[idx[i+total*2]]
			);
    }
	printf("\n");

    int s = 0, ssum = 0;

	ssum = countflag(1);

       
    for (i = 0; i < 50; i++)
    {
        s = findstarter(rand() % ssum);
		gPrevKeys.reset();
        
        int ps = -1;
		gWordNumber = 0;
        while ((gWordCounter.mFlags[s] & 2) == 0 && ps != s && gWordNumber < 500)
        {
            printf("%s ", gWordCounter.mWord[s]);
            ps = s;
            s = nextword(s);            
			gWordNumber++;
        }
        printf("%s ", gWordCounter.mWord[s]);
        printf("\n\n");
        
    }
    
    return 0;
}
